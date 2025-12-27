use core::alloc::Layout;

use arrayvec::ArrayVec;
use ftl_types::error::ErrorCode;
use ftl_utils::alignment::is_aligned;
use ftl_utils::bump_allocator::BumpAllocator;

use crate::address::PAddr;
use crate::address::VAddr;
use crate::arch::MIN_PAGE_SIZE;
use crate::arch::paddr2vaddr;
use crate::arch::vaddr2paddr;
use crate::spinlock::SpinLock;

pub static PAGE_ALLOCATOR: PageAllocator = PageAllocator::new();

struct Mutable {
    regions: ArrayVec<BumpAllocator, 4>,
    usage: Usage,
}

#[derive(Debug, Clone, Copy)]
pub struct Usage {
    /// The total size of memory managed by the allocator (in bytes).
    pub total: usize,
    /// The size of memory that is currently used (in bytes).
    pub used: usize,
}

pub struct PageAllocator {
    mutable: SpinLock<Mutable>,
}

impl PageAllocator {
    pub const fn new() -> Self {
        Self {
            mutable: SpinLock::new(Mutable {
                regions: ArrayVec::new_const(),
                usage: Usage { total: 0, used: 0 },
            }),
        }
    }

    #[allow(unused)]
    pub fn usage(&self) -> Usage {
        self.mutable.lock().usage
    }

    pub fn add_region(&self, paddr: PAddr, len: usize) {
        let mut mutable = self.mutable.lock();

        mutable
            .regions
            .try_push(BumpAllocator::new(paddr.as_usize(), len))
            .expect("too many memory physical memory regions");

        mutable.usage.total += len;
    }

    pub fn allocate_pages_zeroed(&self, len: usize) -> Result<PAddr, ErrorCode> {
        assert!(is_aligned(len, MIN_PAGE_SIZE));

        let layout = Layout::from_size_align(len, MIN_PAGE_SIZE).unwrap();
        let mut mutable = self.mutable.lock();
        for region in mutable.regions.iter_mut() {
            if let Some(paddr) = region.allocate(layout) {
                let paddr = PAddr::new(paddr.get());
                let vaddr = paddr2vaddr(paddr);

                // TODO: More efficient way to zero out the memory (zeroing when
                //       idle?)
                unsafe {
                    core::ptr::write_bytes(vaddr.as_mut_ptr::<u8>(), 0, len);
                }

                mutable.usage.used += len;
                return Ok(paddr);
            }
        }

        Err(ErrorCode::OutOfMemory)
    }

    pub fn free_pages(&self, paddr: PAddr, len: usize) -> Result<(), ErrorCode> {
        assert!(is_aligned(paddr.as_usize(), MIN_PAGE_SIZE));
        assert!(is_aligned(len, MIN_PAGE_SIZE));

        // Bump allocator does not support free operation.
        Ok(())
    }
}

#[repr(align(4096))]
struct PageAligned<T>(T);

/// A temporary RAM area for kernel's global allocator while we scan the
/// device tree and add available RAM regions the allocator.
const EARLY_RAM: PageAligned<[u8; EARLY_RAM_SIZE]> = PageAligned([0; EARLY_RAM_SIZE]);
const EARLY_RAM_SIZE: usize = 64 * 1024; // 64 KB

pub fn init() {
    PAGE_ALLOCATOR.add_region(
        vaddr2paddr(VAddr::new(EARLY_RAM.0.as_ptr() as usize)),
        EARLY_RAM.0.len(),
    );
}
