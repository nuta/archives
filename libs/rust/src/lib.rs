#![feature(lang_items, global_asm, alloc, allocator_api, global_allocator)]
#![no_std]
#![allow(unused_variables)]
#![feature(asm)]

extern crate alloc;
pub use alloc::*;

pub mod allocator;
pub mod interfaces;
pub mod error;
pub mod arch;
pub mod lang_items;
pub mod channel;
pub mod print;

global_asm!(include_str!("start.S"));

#[global_allocator]
static GLOBAL_ALLOCATOR: allocator::MyAllocator = allocator::MyAllocator {};
