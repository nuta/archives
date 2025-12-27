use core::marker::PhantomData;

use ftl::vmarea::VmArea;
use ftl::vmspace::CURRENT_VMSPACE;
use ftl_types::arch::PageAttrs;
use ftl_types::device::IoSpaceDesc;
use ftl_types::error::ErrorCode;
use ftl_utils::alignment::align_down;
use ftl_utils::alignment::align_up;

use crate::barrier::read_barrier;
use crate::barrier::write_barrier;

enum Mapping {
    Memory { base: *mut u8 },
}

pub struct IoSpace {
    size: usize,
    mapping: Mapping,
}

impl IoSpace {
    pub fn map(desc: IoSpaceDesc) -> Result<Self, ErrorCode> {
        let (unaligned_baddr, size) = match desc {
            IoSpaceDesc::Mmio { baddr, size } => (baddr.as_usize(), size),
        };

        let baddr = align_down(unaligned_baddr, 0x1000);
        let aligned_size = align_up(size, 0x1000); // TODO: use MIN_PAGE_SIZE
        let vmarea = VmArea::new_pinned(baddr, aligned_size)?;
        let attrs = PageAttrs::valid().read().write();
        let base_vaddr = CURRENT_VMSPACE.map(&vmarea, 0, 0, aligned_size, attrs)?;

        let offset = unaligned_baddr - baddr;
        let base = unsafe { (base_vaddr as *mut u8).add(offset) };
        Ok(Self {
            size,
            mapping: Mapping::Memory { base },
        })
    }

    pub fn read32<E: Endianess, A: Readable>(&self, reg: DeviceReg<E, A, u32>) -> u32 {
        debug_assert!(reg.offset + size_of::<u32>() <= self.size);

        let raw = match self.mapping {
            Mapping::Memory { base } => unsafe {
                let ptr = base.add(reg.offset) as *const u32;
                core::ptr::read_volatile(ptr)
            },
        };

        // Ensure subsequent memory reads happen after this MMIO read.
        read_barrier();

        E::to_raw32(raw)
    }

    pub fn write32<E: Endianess, A: Writable>(&self, reg: DeviceReg<E, A, u32>, value: u32) {
        debug_assert!(reg.offset + size_of::<u32>() <= self.size);

        // Ensure all prior memory writes complete before MMIO write.
        write_barrier();

        match self.mapping {
            Mapping::Memory { base } => unsafe {
                let ptr = base.add(reg.offset) as *mut u32;
                core::ptr::write_volatile(ptr, E::to_raw32(value))
            },
        }
    }
}

// `IoSpace` can be used across threads (`Send`), but requires exclusively
// locked access (`Sync`).
unsafe impl Send for IoSpace {}

pub trait Endianess {
    fn from_raw32(value: u32) -> u32;
    fn to_raw32(value: u32) -> u32;
}

pub struct LittleEndian;

impl Endianess for LittleEndian {
    fn from_raw32(value: u32) -> u32 {
        u32::from_le(value)
    }

    fn to_raw32(value: u32) -> u32 {
        value.to_le()
    }
}

pub struct ReadOnly;
pub struct WriteOnly;
pub struct ReadWrite;
pub trait Access {}
impl Access for ReadOnly {}
impl Access for WriteOnly {}
impl Access for ReadWrite {}
pub trait Readable: Access {}
pub trait Writable: Access {}
impl Readable for ReadOnly {}
impl Writable for WriteOnly {}
impl Readable for ReadWrite {}
impl Writable for ReadWrite {}

pub struct DeviceReg<E: Endianess, A: Access, T> {
    offset: usize,
    _pd: PhantomData<(E, A, T)>,
}

impl<E: Endianess, A: Access, T> DeviceReg<E, A, T> {
    pub const fn new(offset: usize) -> Self {
        Self {
            offset,
            _pd: PhantomData,
        }
    }
}
