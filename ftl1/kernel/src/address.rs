use core::fmt;

use fmt::Display;

/// A virtual address.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(transparent)]
pub struct VAddr(usize);

impl VAddr {
    pub const fn new(addr: usize) -> Self {
        Self(addr)
    }

    pub const fn add(self, offset: usize) -> Self {
        Self(self.0 + offset)
    }

    pub const fn as_usize(self) -> usize {
        self.0
    }

    pub fn as_mut_ptr<T>(self) -> *mut T {
        self.0 as *mut T
    }
}

/// A user-space address.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(transparent)]
pub struct UAddr(usize);

impl UAddr {
    pub const fn new(addr: usize) -> Self {
        Self(addr)
    }

    pub const fn add(self, offset: usize) -> Self {
        Self(self.0 + offset)
    }

    pub const fn as_usize(self) -> usize {
        self.0
    }

    pub const fn as_vaddr(self) -> VAddr {
        VAddr(self.0)
    }
}

impl From<usize> for UAddr {
    fn from(addr: usize) -> Self {
        Self::new(addr)
    }
}

impl From<UAddr> for usize {
    fn from(addr: UAddr) -> usize {
        addr.as_usize()
    }
}

/// A physical address.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(transparent)]
pub struct PAddr(usize);

impl PAddr {
    pub const fn new(addr: usize) -> Self {
        Self(addr)
    }

    pub const fn as_usize(self) -> usize {
        self.0
    }

    pub fn checked_add(self, offset: usize) -> Option<Self> {
        let paddr = self.0.checked_add(offset)?;
        Some(Self(paddr))
    }
}

impl Display for PAddr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:016x}", self.0)
    }
}
