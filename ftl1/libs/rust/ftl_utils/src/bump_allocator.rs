use core::alloc::Layout;
use core::num::NonZeroUsize;

use crate::alignment::align_down;

/// A bump memory allocator.
///
/// Unlike typical allocators, this allocator does not support freeing memory.
/// Instead, it only supports allocating memory. This makes it extremely fast
/// and simple.
///
/// Typically, this allocator is used for allocating memory in initialization
/// phase such that the allocated memory is never freed.
pub struct BumpAllocator {
    top: usize,
    bottom: usize,
}

impl BumpAllocator {
    pub const fn new(base: usize, len: usize) -> Self {
        Self {
            bottom: base,
            top: base + len,
        }
    }

    /// Allocates `size` bytes of memory with the given `align` bytes alignment.
    /// Returns the beginning address of the allocated memory if successful.
    pub fn allocate(&mut self, layout: Layout) -> Option<NonZeroUsize> {
        if layout.size() == 0 {
            return None;
        }

        let new_top = align_down(self.top.checked_sub(layout.size())?, layout.align());
        if new_top < self.bottom {
            return None;
        }

        self.top = new_top;

        // SAFETY: `self.top` is checked to be larger than `self.bottom`.
        unsafe { Some(NonZeroUsize::new_unchecked(self.top)) }
    }
}

#[cfg(test)]
mod tests {
    use core::num::NonZeroUsize;

    use super::*;

    fn nonzero(value: usize) -> NonZeroUsize {
        NonZeroUsize::new(value).unwrap()
    }

    fn layout(size: usize, align: usize) -> Layout {
        Layout::from_size_align(size, align).unwrap()
    }

    #[test]
    fn test_zero_size() {
        let mut allocator = BumpAllocator::new(0x20000, 0x4000);
        assert_eq!(allocator.allocate(layout(0, 0x1000)), None);
    }

    #[test]
    fn test_bump_allocator() {
        let mut allocator = BumpAllocator::new(0x20000, 0x4000);
        assert_eq!(
            allocator.allocate(layout(0x1000, 0x1000)),
            Some(nonzero(0x23000))
        );
        assert_eq!(
            allocator.allocate(layout(0x1000, 0x1000)),
            Some(nonzero(0x22000))
        );
        assert_eq!(
            allocator.allocate(layout(0xf00, 0x1000)),
            Some(nonzero(0x21000))
        );
        assert_eq!(
            allocator.allocate(layout(0x1000, 0x1000)),
            Some(nonzero(0x20000))
        );
        assert_eq!(allocator.allocate(layout(0x1000, 0x1000)), None);
    }
}
