use core::ops::Add;
pub use core::time::Duration;

use crate::error::ErrorCode;

#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, Debug)]
pub struct Instant {
    // 2^64 nanoseconds = 584 years. Should be enough for most use cases.
    nanoseconds: u64,
}

impl Instant {
    pub const fn from_raw(raw: u64) -> Self {
        Self { nanoseconds: raw }
    }

    pub fn from_usize(raw: usize) -> Result<Self, ErrorCode> {
        let raw_u64 = raw.try_into().map_err(|_| ErrorCode::InvalidArgument)?;
        Ok(Self::from_raw(raw_u64))
    }

    /// Returns an opaque value for [Instant::from_raw].
    pub const fn as_raw(&self) -> u64 {
        self.nanoseconds
    }

    /// Returns the duration since the given instant.
    pub const fn elapsed_since(&self, earlier: Instant) -> Duration {
        Duration::from_nanos(self.nanoseconds.saturating_sub(earlier.nanoseconds))
    }
}

impl Add<Duration> for Instant {
    type Output = Instant;

    fn add(self, rhs: Duration) -> Self::Output {
        let lhs = self.nanoseconds;
        let result = lhs.saturating_add(rhs.as_nanos() as u64);
        Instant::from_raw(result)
    }
}
