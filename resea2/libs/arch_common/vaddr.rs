use core::{fmt, ptr::NonNull};

/// Represents a non-null *kernel* virtual memory address.
#[derive(Debug, Copy, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
#[repr(transparent)]
pub struct VAddr(usize);

impl VAddr {
    /// Constructs VAddr.
    ///
    /// # Safety
    ///
    /// `addr` must points to a virtual memory address accessible from the
    /// kernel. Specifically, it must always be mapped in the page table.
    pub const unsafe fn new(addr: usize) -> VAddr {
        debug_assert!(addr != 0);
        VAddr(addr)
    }

    /// Returns a mutable pointer to a `T`-object at the address.
    ///
    /// # Safety
    ///
    /// The caller should ensure the following:
    ///
    /// - The address is aligned.
    /// - The address points to `T`.
    /// - The pointer is dangled, i.e. prevent use-after-free.
    pub const unsafe fn as_mut_ptr<T>(self) -> *mut T {
        self.0 as *mut _
    }

    /// Returns a `NonNull<T>` pointing to a `T`-object at the address.
    ///
    /// # Safety
    ///
    /// The caller should ensure the following:
    ///
    /// - The address is aligned.
    /// - The address points to `T`.
    /// - The pointer is dangled, i.e. prevent use-after-free.
    pub const unsafe fn as_nonull<T>(self) -> NonNull<T> {
        NonNull::new_unchecked(self.as_mut_ptr())
    }

    #[inline(always)]
    pub const fn as_usize(self) -> usize {
        self.0
    }
}

impl fmt::Display for VAddr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:x}", self.as_usize())
    }
}
