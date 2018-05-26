use core::alloc::{GlobalAlloc, Layout, Opaque};

pub struct MyAllocator {
}

static mut NEXT_MALLOC: u64 = 0xc0000000;

unsafe impl GlobalAlloc for MyAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut Opaque {
        let addr = NEXT_MALLOC;
        NEXT_MALLOC += layout.size() as u64;
        addr as *mut Opaque
    }

    unsafe fn dealloc(&self, ptr: *mut Opaque, layout: Layout) {

    }
}
