//! A physical memory page allocator.
#![cfg_attr(not(test), no_std)]

mod bitmap_allocator;

pub use bitmap_allocator::BitMapAllocator;
