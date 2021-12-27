use core::slice;

use etc::alignment::align_up;
use etc::bitmap::BitMap;

/// A physical memory page allocator.
///
/// This allocator tracks unused pages by a bitmap table at the very beginning of
/// the available memory space passed by [`BitMapAllocator::new`]: Once you allocate
/// a page, the corresponding bit is set. Conversely, once you free a page,
/// the corresponding bit will be unset.
pub struct BitMapAllocator {
    bitmap: BitMap<'static>,
    alloc_base: usize,
    page_size: usize,
}

impl BitMapAllocator {
    /// Constructs a new allocator.
    ///
    /// # Safety
    ///
    /// The caller must ensure that:
    ///
    /// - The range [`base_ptr`, `base_ptr + bitmap_len`] is always accessible until
    ///   this object is destructed.
    /// - `base_ptr` is aligned to `page_size`.
    pub unsafe fn new(
        base_ptr: *mut u8,
        base: usize,
        len: usize,
        page_size: usize,
    ) -> BitMapAllocator {
        let bitmap_len = len / page_size;
        let alloc_base = base + align_up(bitmap_len, page_size);
        let num_chunks = (len - (alloc_base - base)) / page_size;
        BitMapAllocator {
            bitmap: BitMap::new(slice::from_raw_parts_mut(base_ptr, bitmap_len), num_chunks),
            alloc_base,
            page_size,
        }
    }

    #[inline(always)]
    fn bitmap_index_to_addr(&self, index: usize) -> usize {
        self.alloc_base + (index * self.page_size)
    }

    #[inline(always)]
    fn addr_to_bitmap_index(&self, addr: usize) -> usize {
        (addr - self.alloc_base) / self.page_size
    }

    /// Allocates a memory page and returns the address.
    ///
    /// # Time Complexity
    /// `O(|all pages|)` since it scans the whole bitmap table in the worst case.
    pub fn alloc(&mut self, n: usize) -> Option<usize> {
        self.bitmap.find_contiguous_zeros(n).map(|start| {
            self.bitmap.set_contiguous(start, n);
            self.bitmap_index_to_addr(start)
        })
    }

    /// Frees `n` pages beginning from `addr`.
    ///
    /// # Time Complexity
    /// `O(n)`.
    pub fn free(&mut self, addr: usize, n: usize) {
        self.bitmap
            .unset_contiguous(self.addr_to_bitmap_index(addr), n)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn alloc_and_free() {
        const LEN: usize = 0x4000;
        let base = 0x1000_0000;
        let mut allocator = unsafe {
            BitMapAllocator::new(
                Box::leak(Box::new([0; LEN])).as_ptr() as *mut u8,
                base,
                LEN,
                0x1000,
            )
        };

        assert_eq!(allocator.alloc(1), Some(base + 0x1000));
        assert_eq!(allocator.alloc(1), Some(base + 0x2000));
        assert_eq!(allocator.alloc(1), Some(base + 0x3000));
        assert_eq!(allocator.alloc(1), None);
        allocator.free(base + 0x2000, 1);
        assert_eq!(allocator.alloc(1), Some(base + 0x2000));
    }
}
