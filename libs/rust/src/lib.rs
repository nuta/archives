#![feature(lang_items, global_asm, alloc, allocator_api, global_allocator)]
#![no_std]
#![allow(unused_variables)]
#![feature(asm)]

extern crate alloc;
pub use alloc::string;

use core::alloc::{GlobalAlloc, Layout, Opaque};

global_asm!(include_str!("start.S"));

pub struct MyAllocator();
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

#[global_allocator]
static GLOBAL_ALLOCATOR: MyAllocator = MyAllocator {};

#[lang="eh_personality"] extern fn rust_eh_personality() {}
#[lang="panic_fmt"] fn rust_begin_panic() -> ! { loop {} }
#[lang="oom"] extern fn rust_oom() -> ! { loop {} }
