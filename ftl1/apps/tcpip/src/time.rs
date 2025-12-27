use core::time::Duration;

/// Smoltcp expects a raw microsecond precision timestamp, however,
/// `ftl::time::Instant` does not expose it intentionally (i.e.
/// instants are for relative time measurements only).
///
/// This wrapper provides a way to measure absolute time, measured
/// from a certain point in time.
pub struct SmolInstant {
    base: ftl::timer::Instant,
}

impl SmolInstant {
    pub fn new() -> Self {
        let base = ftl::timer::now();
        Self { base }
    }

    pub fn now(&self) -> smoltcp::time::Instant {
        let now = ftl::timer::now();
        let duration = now.elapsed_since(self.base);
        let micros: i64 = duration.as_micros().try_into().unwrap();
        smoltcp::time::Instant::from_micros(micros)
    }

    #[allow(clippy::wrong_self_convention)]
    pub fn from_smol(&self, smol_instant: smoltcp::time::Instant) -> ftl::timer::Instant {
        let micros: u64 = smol_instant.total_micros().try_into().unwrap();
        self.base + Duration::from_micros(micros)
    }
}
