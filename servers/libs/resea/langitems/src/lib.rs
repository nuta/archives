#![feature(lang_items, panic_implementation, global_asm, alloc, allocator_api)]
#![feature(panic_info_message)]
#![no_std]

extern crate core as core_crate;
extern crate resea;
extern crate alloc as alloc_crate;
use core_crate::*;
use core_crate::alloc::Layout;
#[macro_use]
#[allow(unused_imports, unused_attributes)]
use alloc_crate::*;

use resea::interfaces;

mod allocator;

global_asm!(include_str!("start.S"));

#[lang="eh_personality"]
extern fn rust_eh_personality() {
}

#[lang="oom"]
extern fn rust_oom(_layout: Layout) -> ! {
    loop {}
}

#[lang="start"]
fn lang_start<T>(main: fn() -> T, _argc: isize, _argv: *const *const u8) -> isize {
    main();
    0
}

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
