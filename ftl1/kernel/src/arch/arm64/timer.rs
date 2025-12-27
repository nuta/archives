use core::arch::asm;
use core::sync::atomic::AtomicU64;
use core::sync::atomic::Ordering;

use ftl_types::timer::Instant;

use crate::arch::INTERRUPT_CONTROLLER;
use crate::spinlock::SpinLock;

static MULTIPLIER: AtomicU64 = AtomicU64::new(0);

const VIRT_TIMER_IRQ: usize = 27;

pub fn init() {
    let freq: u64;
    unsafe {
        asm!("mrs {}, cntfrq_el0", out(reg) freq);
    }
    let multiplier = 1_000_000_000 / freq;
    MULTIPLIER.store(multiplier, Ordering::Relaxed);

    INTERRUPT_CONTROLLER.acquire(VIRT_TIMER_IRQ).unwrap();
}

pub fn read_monotonic_time() -> Instant {
    let ticks: u64;
    unsafe {
        asm!("mrs {}, cntvct_el0", out(reg) ticks);
    }

    let multiplier = MULTIPLIER.load(Ordering::Relaxed);
    assert!(multiplier > 0, "timer frequency is not set");

    Instant::from_raw(ticks * multiplier)
}

static CURRNET_DEADLINE_TICKS: SpinLock<Option<u64>> = SpinLock::new(None);

pub fn set_timer(deadline: Instant) {
    let multiplier = MULTIPLIER.load(Ordering::Relaxed);
    let ticks = deadline.as_raw() / multiplier;

    // Do not re-arm the timer if the requested deadline is not sooner than
    // the current one.
    let mut current = CURRNET_DEADLINE_TICKS.lock();
    if matches!(&*current, Some(current) if ticks >= *current) {
        return;
    }

    unsafe {
        asm!("msr cntv_cval_el0, {}", in(reg) ticks);
        asm!("msr cntv_ctl_el0, {}", in(reg) 1u64);
    }

    *current = Some(ticks);
}

pub fn acknowledge_timer() {
    unsafe {
        asm!("msr cntv_ctl_el0, {}", in(reg) 0u64);
    }

    *CURRNET_DEADLINE_TICKS.lock() = None;
}
