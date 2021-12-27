use core::ptr::NonNull;

use alloc::alloc::{GlobalAlloc, Layout};

use crate::arch::{TryAsVAddr, PAGE_SIZE};
use arch_common::{bootinfo::BootInfo, paddr::PAddr};
use arrayvec::ArrayVec;
use etc::bytesize::ByteSize;
use heap_allocator::{HeapAllocator, DATA_ALIGN};
use page_allocator::BitMapAllocator;
use spin::Mutex;

static PAGE_AREAS: Mutex<ArrayVec<BitMapAllocator, 8>> = Mutex::new(ArrayVec::new_const());
const NUM_HEAP_EXPANSION_PAGES: usize = 16;

/// Allocates `n` pages.
pub fn alloc_pages(n: usize) -> Option<PAddr> {
    for allocator in PAGE_AREAS.lock().iter_mut() {
        if let Some(paddr) = allocator.alloc(n) {
            return Some(PAddr::new(paddr));
        }
    }

    None
}

struct GlobalAllocator {
    allocator: Mutex<HeapAllocator>,
}

impl GlobalAllocator {
    pub const fn new() -> GlobalAllocator {
        GlobalAllocator {
            allocator: Mutex::new(HeapAllocator::new()),
        }
    }
}

#[cfg_attr(not(feature = "hosting"), global_allocator)]
#[cfg_attr(feature = "hosting", allow(unused))]
static GLOBAL_ALLOCATOR: GlobalAllocator = GlobalAllocator::new();

unsafe impl GlobalAlloc for GlobalAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        debug_assert!(layout.align() <= DATA_ALIGN);

        let mut allocator = self.allocator.lock();
        match allocator.alloc(layout.size()) {
            Some(nonnull) => nonnull.as_ptr(),
            None => {
                // No space left in the kernel heap. Try expanding.
                match alloc_pages(NUM_HEAP_EXPANSION_PAGES) {
                    Some(paddr) => {
                        let vaddr = paddr.try_as_vaddr().expect("unmapped paddr");
                        allocator.insert_heap_area(
                            vaddr.as_nonull(),
                            NUM_HEAP_EXPANSION_PAGES * PAGE_SIZE,
                        );

                        allocator
                            .alloc(layout.size())
                            .expect("run out of memory")
                            .as_ptr()
                    }
                    None => {
                        panic!("run out of memory");
                    }
                }
            }
        }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        debug_assert!(!ptr.is_null());
        self.allocator.lock().free(NonNull::new_unchecked(ptr))
    }
}

#[cfg_attr(not(feature = "hosting"), alloc_error_handler)]
#[cfg_attr(feature = "hosting", allow(dead_code))]
fn alloc_error_handler(_: Layout) -> ! {
    panic!("run out of memory");
}

pub fn init(bootinfo: &BootInfo) {
    {
        let mut page_areas = PAGE_AREAS.lock();
        for area in &bootinfo.free_ram_areas {
            println!(
                "RAM: {:016x} - {:016x} ({})",
                area.paddr.as_usize(),
                area.paddr.as_usize() + area.len,
                ByteSize(area.len)
            );
            page_areas.push(unsafe {
                BitMapAllocator::new(
                    area.vaddr.as_mut_ptr(),
                    area.paddr.as_usize(),
                    area.len,
                    PAGE_SIZE,
                )
            });
        }
    }
}
