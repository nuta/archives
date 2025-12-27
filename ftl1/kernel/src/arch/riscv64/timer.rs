use core::arch::asm;
use core::sync::atomic::AtomicU64;
use core::sync::atomic::Ordering;

use ftl_types::timer::Instant;

use super::sbi;
use crate::spinlock::SpinLock;

static MULTIPLIER: AtomicU64 = AtomicU64::new(0);

pub fn set_frequency(freq: u64) {
    let multiplier = 1_000_000_000 / freq;
    MULTIPLIER.store(multiplier, Ordering::Relaxed);
}

pub fn read_monotonic_time() -> Instant {
    let ticks: u64;
    unsafe {
        asm!("rdtime {}", out(reg) ticks);
    }

    let multiplier = MULTIPLIER.load(Ordering::Relaxed);
    assert!(multiplier > 0, "timer frequency is not set");

    Instant::from_raw(ticks * multiplier)
}

static CURRNET_DEADLINE_TICKS: SpinLock<Option<u64>> = SpinLock::new(None);

pub fn set_timer(deadline: Instant) {
    let mut current = CURRNET_DEADLINE_TICKS.lock();
    let multiplier = MULTIPLIER.load(Ordering::Relaxed);
    let ticks = deadline.as_raw() / multiplier;

    // Do not re-arm the timer if the requested deadline is not sooner than
    // the current one.
    if matches!(&*current, Some(current) if ticks >= *current) {
        return;
    }

    sbi::set_timer(ticks);
    *current = Some(ticks);
}

pub fn acknowledge_timer() {
    sbi::set_timer(0xffff_ffff_ffff_ffff);
    *CURRNET_DEADLINE_TICKS.lock() = None;
}
