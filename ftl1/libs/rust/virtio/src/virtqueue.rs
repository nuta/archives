use ftl::prelude::vec::Vec;
use ftl_driver::device::BusAddr;
use ftl_driver::device::DmaDesc;
use ftl_driver::persistent_dma::PersistentDma;
use ftl_utils::endianess::LE;

use crate::free_list::FreeList;
use crate::transports::Transport;

const VIRTQ_DESC_F_NEXT: u16 = 1;
const VIRTQ_DESC_F_WRITE: u16 = 2;

pub const VIRTQ_SIZE: usize = 64;

#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct VirtqDesc {
    pub addr: LE<u64>,
    pub len: LE<u32>,
    pub flags: LE<u16>,
    pub next: LE<u16>,
}

impl VirtqDesc {
    pub fn is_writable(&self) -> bool {
        let flags: u16 = self.flags.into();
        flags & VIRTQ_DESC_F_WRITE != 0
    }

    pub fn has_next(&self) -> bool {
        let flags: u16 = self.flags.into();
        flags & VIRTQ_DESC_F_NEXT != 0
    }
}

#[repr(C)]
struct VirtqAvail<const N: usize> {
    flags: LE<u16>,
    index: LE<u16>,
    ring: [LE<u16>; N],
}

#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct VirtqUsedElem {
    id: LE<u32>,
    len: LE<u32>,
}

#[repr(C)]
struct VirtqUsed<const N: usize> {
    flags: LE<u16>,
    index: LE<u16>,
    ring: [VirtqUsedElem; N],
}

#[derive(Debug)]
pub enum DescBuffer {
    ReadOnlyFromDevice { baddr: BusAddr, len: u32 },
    WritableFromDevice { baddr: BusAddr, len: u32 },
}

pub struct UsedChain {
    pub descs: Vec<DescBuffer>,
    pub total_len: usize,
}

#[derive(Debug)]
pub enum Error {
    DmaAlloc(ftl_driver::persistent_dma::Error),
    TooSmallQueueSize(u16),
    OutOfDescriptors,
    BufferTooLarge(usize),
}

pub struct VirtQueue {
    desc_table: PersistentDma<[VirtqDesc; VIRTQ_SIZE]>,
    avail_ring: PersistentDma<VirtqAvail<VIRTQ_SIZE>>,
    used_ring: PersistentDma<VirtqUsed<VIRTQ_SIZE>>,
    index: u16,
    last_used_index: u16,
    num_total_descs: u16,
    free_list: FreeList<u16>,
}

impl VirtQueue {
    pub(crate) fn new(
        index: u16,
        dma_desc: DmaDesc,
        transport: &dyn Transport,
    ) -> Result<VirtQueue, Error> {
        transport.select_queue(index);

        let queue_max_size = transport.queue_max_size();
        if queue_max_size < VIRTQ_SIZE as u16 {
            return Err(Error::TooSmallQueueSize(queue_max_size));
        }
        transport.set_queue_size(VIRTQ_SIZE as u16);

        let desc_table: PersistentDma<[VirtqDesc; VIRTQ_SIZE]> =
            PersistentDma::alloc(dma_desc).map_err(Error::DmaAlloc)?;
        let avail_ring = PersistentDma::alloc(dma_desc).map_err(Error::DmaAlloc)?;
        let used_ring = PersistentDma::alloc(dma_desc).map_err(Error::DmaAlloc)?;
        transport.set_queue_desc_paddr(desc_table.baddr());
        transport.set_queue_driver_paddr(avail_ring.baddr());
        transport.set_queue_device_paddr(used_ring.baddr());
        transport.enable_queue();

        // Enqueue the free list.
        let mut free_list = FreeList::new();
        for i in 0..VIRTQ_SIZE {
            free_list.push(i as u16);
        }

        Ok(VirtQueue {
            desc_table,
            avail_ring,
            used_ring,
            index,
            last_used_index: 0,
            free_list,
            num_total_descs: VIRTQ_SIZE as u16,
        })
    }

    pub fn push_avail(&mut self, chain: &[DescBuffer]) -> Result<(), Error> {
        assert!(!chain.is_empty());

        let mut descs = self.desc_table.write_guard();
        let Some(desc_indices) = self.free_list.pop(chain.len()) else {
            return Err(Error::OutOfDescriptors);
        };

        for (i, buffer) in chain.iter().enumerate() {
            let (baddr, len, flags) = match buffer {
                DescBuffer::ReadOnlyFromDevice { baddr, len } => (*baddr, *len, 0),
                DescBuffer::WritableFromDevice { baddr, len } => (*baddr, *len, VIRTQ_DESC_F_WRITE),
            };

            let desc = &mut descs[desc_indices[i] as usize];
            desc.addr = (baddr.as_usize() as u64).into();
            desc.len = len.into();

            let mut desc_flags = flags;

            if i < chain.len() - 1 {
                desc.next = desc_indices[i + 1].into();
                desc_flags |= VIRTQ_DESC_F_NEXT;
            } else {
                desc.next = 0.into();
            }

            desc.flags = desc_flags.into();
        }

        let mut avail = self.avail_ring.write_guard();
        let avail_index: u16 = avail.index.into();
        let avail_elem_index = avail_index % self.num_total_descs;
        avail.ring[avail_elem_index as usize] = desc_indices[0].into();
        avail.index = avail_index.wrapping_add(1).into();
        Ok(())
    }

    /// Notifies the device to start processing descriptors.
    pub fn notify(&self, transport: &mut dyn Transport) {
        transport.notify_queue(self.index);
    }

    /// Returns a chain of descriptors processed by the device.
    pub fn pop_used(&mut self) -> Option<UsedChain> {
        let used = self.used_ring.read_guard();
        let used_index: u16 = used.index.into();
        if self.last_used_index == used_index {
            return None;
        }

        let head = used.ring[(self.last_used_index % self.num_total_descs) as usize];
        self.last_used_index = self.last_used_index.wrapping_add(1);

        let mut index: u32 = head.id.into();
        let mut chain = Vec::new();
        let descs = self.desc_table.read_guard();
        loop {
            let desc = &descs[index as usize];
            let raw_addr: u64 = desc.addr.into();
            let baddr = BusAddr::new(raw_addr as usize);
            let len: u32 = desc.len.into();
            let buffer = if desc.is_writable() {
                DescBuffer::WritableFromDevice { baddr, len }
            } else {
                DescBuffer::ReadOnlyFromDevice { baddr, len }
            };

            chain.push(buffer);
            self.free_list.push(index as u16);

            if !desc.has_next() {
                break;
            }

            let next_index: u16 = desc.next.into();
            index = next_index as u32;
        }

        let total_len: u32 = head.len.into();
        Some(UsedChain {
            total_len: total_len as usize,
            descs: chain,
        })
    }
}
