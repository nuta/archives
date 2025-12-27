use core::fmt;
use core::ops::BitAnd;
use core::ops::BitAndAssign;
use core::ops::BitOr;
use core::ops::BitOrAssign;
use core::ops::Not;

use crate::error::ErrorCode;
use crate::handle::HandleId;

#[derive(Clone, Copy, PartialEq, Eq)]
pub struct Readiness(u8);

impl Readiness {
    pub const PEER_CLOSED: Self = Readiness(1 << 0);
    pub const READABLE: Self = Readiness(1 << 1);
    pub const WRITABLE: Self = Readiness(1 << 2);

    pub const NONE: Self = Readiness(0);
    pub const ALL: Self = Readiness(Self::PEER_CLOSED.0 | Self::READABLE.0 | Self::WRITABLE.0);

    pub const fn from_raw(raw: u8) -> Result<Self, ErrorCode> {
        Ok(Readiness(raw))
    }

    pub const fn empty() -> Self {
        Readiness(0)
    }

    pub fn as_usize(&self) -> usize {
        self.0 as usize
    }

    pub fn is_empty(&self) -> bool {
        self.0 == 0
    }

    pub fn contains(&self, other: Self) -> bool {
        (self.0 & other.0) == other.0
    }

    pub fn intersection(&self, other: Self) -> Self {
        Readiness(self.0 & other.0)
    }
}

impl fmt::Debug for Readiness {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut list = f.debug_list();

        if self.contains(Self::PEER_CLOSED) {
            list.entry(&"PEER_CLOSED");
        }
        if self.contains(Self::READABLE) {
            list.entry(&"READABLE");
        }
        if self.contains(Self::WRITABLE) {
            list.entry(&"WRITABLE");
        }

        list.finish()
    }
}

impl Not for Readiness {
    type Output = Readiness;

    fn not(self) -> Self::Output {
        Readiness(!self.0)
    }
}

impl BitOr for Readiness {
    type Output = Readiness;

    fn bitor(self, rhs: Self) -> Self::Output {
        Readiness(self.0 | rhs.0)
    }
}

impl BitAnd for Readiness {
    type Output = Readiness;

    fn bitand(self, rhs: Self) -> Self::Output {
        Readiness(self.0 & rhs.0)
    }
}

impl BitOrAssign for Readiness {
    fn bitor_assign(&mut self, rhs: Self) {
        self.0 |= rhs.0;
    }
}

impl BitAndAssign for Readiness {
    fn bitand_assign(&mut self, rhs: Self) {
        self.0 &= rhs.0;
    }
}

pub struct PollEvent {
    pub handle_id: HandleId,
    pub readiness: Readiness,
}

impl PollEvent {
    pub const fn new(handle_id: HandleId, readiness: Readiness) -> Self {
        PollEvent {
            handle_id,
            readiness,
        }
    }

    pub fn as_raw(&self) -> usize {
        let events = self.readiness.as_usize();
        let handle_id = self.handle_id.as_usize();
        debug_assert!(handle_id < 1 << 28, "handle_id too large");
        (handle_id << 4) | events
    }

    pub fn from_raw(raw: usize) -> Result<Self, ErrorCode> {
        let handle_id = HandleId::from_usize(raw >> 4)?;
        let raw_readiness = raw & Readiness::ALL.as_usize();
        let events = Readiness::from_raw(raw_readiness as u8)?;
        Ok(PollEvent::new(handle_id, events))
    }
}
