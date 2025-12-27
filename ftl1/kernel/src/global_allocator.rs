use core::alloc::Layout;

use ftl_utils::byte_size::ByteSize;
use talc::OomHandler;
use talc::Span;
use talc::Talc;
use talc::Talck;

use crate::arch::paddr2vaddr;
use crate::memory::PAGE_ALLOCATOR;

const CHUNK_SIZE: usize = 64 * 1024;

struct AllocateOnOom {
    _private: (),
}

impl AllocateOnOom {
    pub const fn new() -> Self {
        Self { _private: () }
    }
}

impl OomHandler for AllocateOnOom {
    fn handle_oom(talc: &mut Talc<Self>, layout: Layout) -> Result<(), ()> {
        assert!(
            // Divided by 2 to leave some space for Talc's metadata.
            layout.size() <= CHUNK_SIZE / 2,
            "too large memory allocation: {}",
            ByteSize(layout.size())
        );

        trace!(
            "extending kernel heap: requested {}, supplying {}",
            ByteSize(layout.size()),
            ByteSize(CHUNK_SIZE)
        );

        let paddr = PAGE_ALLOCATOR
            .allocate_pages_zeroed(CHUNK_SIZE)
            .expect("out of memory");
        let ptr: *mut u8 = paddr2vaddr(paddr).as_mut_ptr();
        let end = unsafe { ptr.add(CHUNK_SIZE) };

        unsafe {
            talc.claim(Span::new(ptr, end))
                .expect("failed to extend the kernel heap");
        }

        Ok(())
    }
}

#[cfg_attr(target_os = "none", global_allocator)]
static GLOBAL_ALLOCATOR: Talck<spin::Mutex<()>, AllocateOnOom> = {
    let oom_handler = AllocateOnOom::new();
    let talc = Talc::new(oom_handler);

    talc.lock()
};
