#![feature(lang_items, panic_implementation, global_asm, alloc, allocator_api, global_allocator)]
#![feature(pattern_parentheses)]
#![feature(panic_info_message)]
#![no_std]
#![allow(unused_variables)]
#![feature(asm)]

extern crate alloc;
pub use alloc::*;
pub use core::*;

#[inline(always)]
pub fn align(x: usize, align: usize) -> usize {
    (x + align - 1) & !(align - 1)
}

pub mod interfaces;
pub mod allocator;
pub mod arch;
pub mod lang_items;
pub mod channel;
pub mod print;
pub mod server;

global_asm!(include_str!("start.S"));

#[global_allocator]
static GLOBAL_ALLOCATOR: allocator::MyAllocator = allocator::MyAllocator {};


#[panic_implementation]
fn panic(info: &panic::PanicInfo) -> ! {
    let msg = if let Some(message) = info.message() {
        format!("panic: {:}\n", message)
    } else {
        format!("panic: (without message)\n")
    };

    interfaces::logging::Logging::from_cid(1).emit(msg.as_bytes()).ok();
    loop {}
}
