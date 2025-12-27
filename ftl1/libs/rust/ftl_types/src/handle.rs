use core::ops::BitOr;

use crate::error::ErrorCode;

/// A handle ID.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct HandleId(i32);

impl HandleId {
    /// Creates a handle ID from a raw integer.
    pub const fn from_raw(raw: i32) -> HandleId {
        debug_assert!(raw >= 0);
        HandleId(raw)
    }

    pub fn from_usize(raw: usize) -> Result<HandleId, ErrorCode> {
        let raw_i32 = raw.try_into().map_err(|_| ErrorCode::InvalidArgument)?;
        Ok(HandleId(raw_i32))
    }

    pub const fn as_usize(&self) -> usize {
        self.0 as usize
    }
}

/// Allowed operations on a handle.
#[derive(Clone, Copy, PartialEq, Eq, Hash)]
pub struct HandleRight(pub u8);

impl HandleRight {
    pub const READ: HandleRight = HandleRight(1 << 0);
    pub const WRITE: HandleRight = HandleRight(1 << 1);
    pub const ALL: HandleRight = HandleRight(Self::READ.0 | Self::WRITE.0);

    pub fn contains(&self, other: HandleRight) -> bool {
        (self.0 & other.0) == other.0
    }
}

impl BitOr for HandleRight {
    type Output = HandleRight;

    fn bitor(self, rhs: Self) -> Self::Output {
        HandleRight(self.0 | rhs.0)
    }
}
