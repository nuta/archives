use core::fmt;

/// Represents a physical memory address.
#[derive(Debug, Copy, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
#[repr(transparent)]
pub struct PAddr(usize);

impl PAddr {
    pub const fn new(addr: usize) -> PAddr {
        PAddr(addr)
    }

    #[inline(always)]
    pub const fn as_usize(self) -> usize {
        self.0
    }
}

impl fmt::Display for PAddr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:x}", self.as_usize())
    }
}
