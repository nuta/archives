#![no_std]

#[macro_use]
extern crate resea;
use resea::option::Option;
use resea::align;
use resea::sync;
use resea::mem::size_of;
use resea::arch::x64::{pmalloc, IoPort};

const VIRTIO_QUEUE_NUM_MAX: usize = 4;

const VIRTIO_STATUS_ACK: u8          = 1;
const VIRTIO_STATUS_DRIVER: u8       = 2;
const VIRTIO_STATUS_DRIVER_OK: u8    = 4;
const VIRTIO_STATUS_FEATURES_OK: u8  = 8;

const VIRTIO_IOSPACE_LENGTH: u16 = 0x40;
const IO_DEVICE_FEATS: u16    = 0x00;
const IO_GUEST_FEATS: u16     = 0x04;
const IO_DEVICE_STATUS: u16   = 0x12;
const IO_DEVICE_SPECIFIC: u16 = 0x14;
const IO_QUEUE_SELECT: u16    = 0x0e;
const IO_QUEUE_ADDR: u16      = 0x08;
const IO_QUEUE_NOTIFY: u16    = 0x10;
const IO_QUEUE_SIZE: u16      = 0x0c;
const QUEUE_ALIGN: usize = 0x1000;

pub const VIRTIO_DESC_F_NEXT: u16 =  1;
pub const VIRTIO_DESC_F_READ_ONLY: u16 = 0;
pub const VIRTIO_DESC_F_WRITE_ONLY: u16 = 2; /* write-only from the host  */

#[repr(packed)]
struct VirtioDesc {
    addr: u64,
    len: u32,
    flags: u16,
    next: u16,
}

#[repr(packed)]
struct VirtioUsedRing {
    index: u32,
    len: u32,
}

#[repr(packed)]
struct VirtioAvailRing {
    index: u16
}

#[repr(packed)]
struct VirtioAvail {
  flags: u16,
  index: u16
}

#[repr(packed)]
struct VirtioUsed {
  flags: u16,
  index: u16,
}

#[derive(Clone, Copy)]
struct VirtioQueue {
  desc: *mut VirtioDesc,
  avail: *mut VirtioAvail,
  avail_ring: *mut VirtioAvailRing,
  used: *mut VirtioUsed,
  used_ring: *mut VirtioUsedRing,
  index: u8,
  last_used_index: u16,
  queue_num: usize
}

pub struct Virtio {
    ioport: IoPort,
    queues: [Option<VirtioQueue>; VIRTIO_QUEUE_NUM_MAX]
}

pub struct VirtioRequest {
    pub data: usize,
    pub len: usize,
    pub flags: u16,
}

impl Virtio {
    pub fn new(iobase: u16) -> Virtio {
        Virtio {
            ioport: IoPort::new(iobase, VIRTIO_IOSPACE_LENGTH),
            queues: [None; VIRTIO_QUEUE_NUM_MAX]
        }
    }

    pub unsafe fn setup(&self) {
        /* Reset the device. */
        self.ioport.out8(IO_DEVICE_STATUS, 0x00);
        self.ioport.in8(IO_DEVICE_STATUS);

        /* Tell the device that we know how to use it. */
        self.ioport.out8(IO_DEVICE_STATUS, VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER);
    }

    pub unsafe fn get_features(&self) -> u32 {
        self.ioport.in32(IO_DEVICE_FEATS)
    }

    pub unsafe fn set_features(&self, features: u32) {
        self.ioport.out32(IO_GUEST_FEATS, features);
        let old = self.ioport.in8(IO_DEVICE_STATUS);
        self.ioport.out8(IO_DEVICE_STATUS, VIRTIO_STATUS_FEATURES_OK | old);
    }

    pub unsafe fn activate(&self) {
        let old = self.ioport.in8(IO_DEVICE_STATUS);
        self.ioport.out8(IO_DEVICE_STATUS, VIRTIO_STATUS_DRIVER_OK | old);
    }

    pub unsafe fn setup_queue(&mut self, queue_index: u8) {
        /* Get the number of queue. */
        self.ioport.out16(IO_QUEUE_SELECT, queue_index);
        let queue_num = self.ioport.in16(IO_QUEUE_SIZE) as usize;

        if queue_num == 0 {
            // TODO: return an error
            return;
        }

        let desc_size = size_of::<VirtioDesc>() * queue_num;
        let avail_size = size_of::<VirtioAvail>() + size_of::<VirtioAvailRing>() * queue_num;
        let used_size = size_of::<VirtioUsed>() + size_of::<VirtioUsedRing>() * queue_num;
        let queue_size = align(desc_size + avail_size, QUEUE_ALIGN)
                       + used_size;

        let (mut vaddr, paddr) = pmalloc(0, 0, queue_size);

        let desc = vaddr as *mut VirtioDesc;
        vaddr += size_of::<VirtioDesc>() * queue_num;
        let avail = vaddr as *mut VirtioAvail;
        vaddr += size_of::<VirtioAvail>();
        let avail_ring = vaddr as *mut VirtioAvailRing;
        vaddr = align(vaddr + size_of::<VirtioAvailRing>() * queue_num, QUEUE_ALIGN);
        let used = vaddr as *mut VirtioUsed;
        vaddr += size_of::<VirtioUsed>();
        let used_ring = vaddr as *mut VirtioUsedRing;

        (*avail).index = 0;
        (*avail).flags = 0;
        (*used).index = 0;
        (*used).flags = 0;

        /* tell the device the physical address of the queue */
        assert!(paddr <= 0xffffffff);
        self.ioport.out32(IO_QUEUE_ADDR, paddr as u32 >> 12);

        self.queues[queue_index as usize] = Some(VirtioQueue {
            desc: desc,
            avail: avail,
            avail_ring: avail_ring,
            used: used,
            used_ring: used_ring,
            queue_num: queue_num,
            index: queue_index,
            last_used_index: 0
        });
    }

    pub fn request(&self, queue_index: u8, rs: &[VirtioRequest]) {
        let queue = self.queues[queue_index as usize].unwrap();
        unsafe {
            queue.enqueue_request(rs);

            let used_index = (*queue.used).index;

            /* notify the device */
            println!("kicking");
            self.ioport.out16(IO_QUEUE_NOTIFY, queue.index);

            /* Wait for completion. TODO: use interrupt */
            // while (*queue.used).index == used_index {};
        }
    }
}

impl VirtioQueue {
    fn alloc_desc(&self) -> Option<usize> {
        /* allocate a virtio_desc */
        for i in 1..(self.queue_num) {
            unsafe {
                let mut desc = self.desc.offset(i as isize);
                if (*desc).len == 0 {
                    (*desc).len = 1; // mark as reserved
                    println!("ok, got a desc #{}", i);
                    return Some(i); // TODO: lock
                }
            }
        }

        None
    }


    pub fn enqueue_request(&self, rs: &[VirtioRequest]) {
        let mut desc_index = self.alloc_desc().unwrap();
        let first_desc_index = desc_index;

        unsafe {
            println!("rs #: {}", rs.len());
            for i in 0..(rs.len()) {
                println!("#{}: addr={:x}, len={:x}, flags={:x}", desc_index, rs[i].data, rs[i].len, rs[i].flags);
                let desc = self.desc.offset(desc_index as isize);

                let mut next_desc_index = 0;
                if i == rs.len() - 1 {
                  // the final desc
                  next_desc_index = 0;
                  (*desc).flags = rs[i].flags;
                } else {
                  next_desc_index = self.alloc_desc().unwrap();
                  (*desc).flags = rs[i].flags | VIRTIO_DESC_F_NEXT;
                }

                (*desc).addr = rs[i].data as u64;
                (*desc).len  = rs[i].len as u32;
                (*desc).next = next_desc_index as u16;
                desc_index = next_desc_index;
            }

            /* append the index of the first queue_desc to avail ring */
            let avail_elem = self.avail_ring.offset((*self.avail).index as isize % self.queue_num as isize);
            (*avail_elem).index = first_desc_index as u16;

            /* increment the index in avail_ring */
            sync::atomic::fence(sync::atomic::Ordering::Release);
            (*self.avail).index += 1;
        }
    }
}
