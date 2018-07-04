#![feature(lang_items, panic_implementation, global_asm, alloc, allocator_api, global_allocator)]
#![feature(pattern_parentheses)]
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

pub mod allocator;
pub mod interfaces;
pub mod error;
pub mod arch;
pub mod lang_items;
pub mod channel;
pub mod print;
pub mod server;

global_asm!(include_str!("start.S"));

#[global_allocator]
static GLOBAL_ALLOCATOR: allocator::MyAllocator = allocator::MyAllocator {};
