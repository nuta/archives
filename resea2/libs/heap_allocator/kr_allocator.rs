use core::mem::size_of;
use core::ptr::NonNull;

use collections::{
    define_list_node,
    linked_list::{List, ListLink},
};
use etc::alignment::{align_up, is_aligned};

const HEADER_SIZE: usize = size_of::<AllocChunk>();
pub const DATA_ALIGN: usize = 32;
const MAGIC_FREE: u64 = 0xdead_babe_dead_babe; // 16045686376333425342 in decimal
const MAGIC_IN_USE: u64 = 0x7979_babe_7979_babe; // 8753232678015843006 in decimal

/// The header of an allocation.
#[repr(align(32))]
struct AllocChunk {
    // The magic value to check double-free.
    magic: u64,
    /// The maximum length of the data area. The length of an allocation is
    /// less than or equal to the capacity.
    capacity: usize,
    /// The free list link.
    link: ListLink<AllocChunkNode>,
    // repr(align(32)) adds padding here. Immediately after the padding, the
    // data area comes.
}

impl AllocChunk {
    pub unsafe fn new_at(ptr: NonNull<u8>, len: usize) -> NonNull<AllocChunk> {
        assert!(len > HEADER_SIZE);
        // SAFETY: We have the ownership of the memory chunk at `ptr`.
        let mut ptr = ptr.cast::<AllocChunk>();
        *ptr.as_mut() = AllocChunk {
            magic: MAGIC_FREE,
            capacity: len - HEADER_SIZE,
            link: ListLink::empty(),
        };
        ptr
    }

    pub unsafe fn from_data_ptr(data_ptr: NonNull<u8>) -> NonNull<AllocChunk> {
        debug_assert!(is_aligned(data_ptr.as_ptr() as usize, DATA_ALIGN));
        NonNull::new_unchecked(data_ptr.as_ptr().sub(HEADER_SIZE) as *mut AllocChunk)
    }

    pub fn data(&self) -> NonNull<u8> {
        // SAFETY: `self` is never NULL.
        let ptr = unsafe { NonNull::new_unchecked((self as *const _ as *mut u8).add(HEADER_SIZE)) };
        debug_assert!(is_aligned(ptr.as_ptr() as usize, DATA_ALIGN));
        ptr
    }
}

define_list_node!(AllocChunkNode, NonNull<AllocChunk>, link);

pub struct KrAllocator {
    free_list: List<AllocChunkNode>,
}

impl KrAllocator {
    pub const fn new() -> KrAllocator {
        KrAllocator {
            free_list: List::new(),
        }
    }

    pub fn insert_heap_area(&mut self, ptr: NonNull<u8>, len: usize) {
        // SAFETY: If the insertion fails, other thread is trying to register
        // the same memory chunk.
        debug_assert!(is_aligned(ptr.as_ptr() as usize, DATA_ALIGN));
        self.free_list
            .push_back(unsafe { AllocChunk::new_at(ptr, len) })
            .unwrap();
    }

    pub fn alloc(&mut self, len: usize) -> Option<NonNull<u8>> {
        let len = align_up(len, DATA_ALIGN);

        let chunk = match self
            .free_list
            .remove_first_if(|chunk| len <= chunk.capacity)
        {
            Some(mut chunk) => unsafe { chunk.as_mut() },
            None => return None,
        };

        // `chunk` points to an unused space larger than `len`. Try splitting it
        // if possible and return the pointer to its data area.

        assert_eq!(chunk.magic, MAGIC_FREE);
        chunk.magic = MAGIC_IN_USE;

        if chunk.capacity > len + HEADER_SIZE {
            // Large enough to be splitted.
            let new_chunk = unsafe {
                let ptr = NonNull::new_unchecked(chunk.data().as_ptr().add(len));
                AllocChunk::new_at(ptr, chunk.capacity - len)
            };

            chunk.capacity = len;
            // SAFETY: No other threads will have the access to the popped chunk.
            self.free_list.push_back(new_chunk).unwrap();
        }

        Some(chunk.data())
    }

    pub fn free(&mut self, ptr: NonNull<u8>) {
        let chunk_ptr = unsafe {
            let mut chunk_ptr = AllocChunk::from_data_ptr(ptr);
            let chunk = chunk_ptr.as_mut();
            assert_eq!(chunk.magic, MAGIC_IN_USE);
            chunk.magic = MAGIC_FREE;
            chunk_ptr
        };

        // SAFETY: If the insertion fails, other thread is trying to free
        // the same memory chunk. Namely, double-free.
        self.free_list.push_back(chunk_ptr).unwrap();
    }
}

unsafe impl Send for KrAllocator {}
unsafe impl Sync for KrAllocator {}

#[cfg(test)]
mod tests {
    use super::*;
    use std::alloc::{GlobalAlloc, Layout, System};

    trait Addoffset {
        fn offset(&self, offset: usize) -> Self;
    }

    impl<T> Addoffset for NonNull<T> {
        fn offset(&self, offset: usize) -> Self {
            unsafe {
                let ptr: *mut u8 = self.as_ptr() as *mut _;
                NonNull::new_unchecked(ptr.add(offset) as *mut T)
            }
        }
    }

    fn alloc_heap(len: usize) -> NonNull<u8> {
        unsafe {
            NonNull::new(System.alloc(Layout::from_size_align(len, 16).unwrap()))
                .expect("failed to allocate a heap area for testing")
        }
    }

    #[test]
    fn header_size_is_aligned() {
        assert!(is_aligned(size_of::<AllocChunk>(), DATA_ALIGN));
    }

    #[test]
    fn alloc_and_free() {
        let mut allocator = KrAllocator::new();
        let base = alloc_heap(1024);
        allocator.insert_heap_area(base, 1024);
        let ptr_512 = base.offset(HEADER_SIZE);
        let ptr_256 = base.offset(HEADER_SIZE + 512 + HEADER_SIZE);
        let ptr_64 = base.offset(HEADER_SIZE + 512 + HEADER_SIZE + 256 + HEADER_SIZE);
        assert_eq!(allocator.alloc(512), Some(ptr_512));
        assert_eq!(allocator.alloc(256), Some(ptr_256));
        assert_eq!(allocator.alloc(64), Some(ptr_64));
        assert_eq!(allocator.alloc(256), None);

        allocator.free(ptr_512);
        let ptr_300 = base.offset(HEADER_SIZE);
        assert_eq!(allocator.alloc(300), Some(ptr_300));
    }
}
