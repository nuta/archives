use core::ptr;
use core::ptr::NonNull;

use ftl::prelude::*;
use ftl::vmarea::VmArea;
use ftl::vmspace::CURRENT_VMSPACE;
use ftl_types::arch::PageAttrs;
use ftl_types::error::ErrorCode;
use ftl_utils::alignment::align_up;
use hashbrown::HashMap;

use crate::barrier::read_barrier;
use crate::barrier::write_barrier;
use crate::device::BusAddr;
use crate::device::DmaDesc;

/// Number of DMA slots
const NUM_SLOTS: usize = 128;

/// Size per slot (8 KB)
const SLOT_SIZE: usize = 8 * 1024;

#[derive(Debug)]
pub enum Error {
    VmAreaAlloc(ErrorCode),
    VmSpaceMap(ErrorCode),
    NoFreeSlots,
    MappingNotFound,
    BufferTooLarge,
}

pub enum Direction {
    ToDevice,
    FromDevice,
}

pub trait DmaBuffer {
    fn as_slice_mut(&mut self) -> &mut [u8];
}

impl DmaBuffer for Vec<u8> {
    fn as_slice_mut(&mut self) -> &mut [u8] {
        self.as_mut_slice()
    }
}

struct Slot {
    _vmarea: VmArea,
    ptr: NonNull<u8>,
    baddr: BusAddr,
}

struct Inflight<Buf> {
    slot_idx: usize,
    buf: Buf,
    len: usize,
}

/// Oneshot DMA transfer, so-called *streaming DMA*.
pub struct OneshotDma<Buf: DmaBuffer> {
    slots: Vec<Slot>,
    free_slots: Vec<usize>,
    inflight: HashMap<BusAddr, Inflight<Buf>>,
}

impl<Buf: DmaBuffer> OneshotDma<Buf> {
    pub fn new(desc: DmaDesc) -> Result<Self, Error> {
        match desc {
            DmaDesc::Direct => {
                let size = align_up(SLOT_SIZE, 0x1000); // TODO: use MIN_PAGE_SIZE
                let mut slots = Vec::with_capacity(NUM_SLOTS);

                // Pre-allocate all slots
                for _ in 0..NUM_SLOTS {
                    let (vmarea, paddr) =
                        VmArea::alloc_contiguous(size).map_err(Error::VmAreaAlloc)?;
                    let attrs = PageAttrs::valid().read().write();
                    let vaddr = CURRENT_VMSPACE
                        .map(&vmarea, 0, 0, size, attrs)
                        .map_err(Error::VmSpaceMap)?;
                    let baddr = BusAddr::new(paddr);
                    slots.push(Slot {
                        _vmarea: vmarea,
                        ptr: NonNull::new(vaddr as *mut u8).unwrap(),
                        baddr,
                    });
                }

                let free_slots = (0..NUM_SLOTS).collect();

                Ok(Self {
                    slots,
                    free_slots,
                    inflight: HashMap::new(),
                })
            }
        }
    }

    pub fn map(&mut self, mut buf: Buf, direction: Direction) -> Result<BusAddr, Error> {
        let buf_slice = buf.as_slice_mut();
        let len = buf_slice.len();

        if len > SLOT_SIZE {
            return Err(Error::BufferTooLarge);
        }

        let slot_idx = self.free_slots.pop().ok_or(Error::NoFreeSlots)?;
        let Slot { ptr, baddr, .. } = &self.slots[slot_idx];

        if matches!(direction, Direction::ToDevice) {
            write_barrier();
            unsafe {
                ptr::copy_nonoverlapping(buf_slice.as_ptr(), ptr.as_ptr(), len);
            }
        }

        self.inflight
            .insert(*baddr, Inflight { slot_idx, buf, len });

        Ok(*baddr)
    }

    pub fn unmap(&mut self, baddr: BusAddr, direction: Direction) -> Result<Buf, Error> {
        match direction {
            Direction::FromDevice => read_barrier(),
            Direction::ToDevice => {}
        }

        let info = self.inflight.remove(&baddr).ok_or(Error::MappingNotFound)?;

        let mut buf = info.buf;
        if matches!(direction, Direction::FromDevice) {
            let Slot { ptr, .. } = &self.slots[info.slot_idx];
            unsafe {
                ptr::copy_nonoverlapping(ptr.as_ptr(), buf.as_slice_mut().as_mut_ptr(), info.len);
            }
        }

        self.free_slots.push(info.slot_idx);

        Ok(buf)
    }
}

unsafe impl<Buf: DmaBuffer + Send> Send for OneshotDma<Buf> {}
