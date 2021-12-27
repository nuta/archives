#![cfg_attr(not(test), no_std)]

mod kr_allocator;

pub use kr_allocator::{KrAllocator as HeapAllocator, DATA_ALIGN};
