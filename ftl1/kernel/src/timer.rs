//! Timer management.
//!
//! # Lock Ordering
//!
//! You must acquire the locks in the following order:
//!
//! 1. `TIMERS`
//! 2. `Timer.mutable`

use alloc::collections::btree_map::BTreeMap;
use alloc::vec::Vec;
use core::mem::size_of;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::handle::HandleRight;
use ftl_types::poll::Readiness;
use ftl_types::timer::Instant;
use ftl_utils::static_assert;

use crate::arch;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::poll::PollEmitter;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;
use crate::thread::Thread;

/// A mapping from deadline to active timers.
///
/// `BTreeMap` is for sorting the timers by deadline, to find the most imminent
/// timer efficiently.
///
/// `Vec` is for storing the timers with the same deadline.
pub static TIMERS: SpinLock<BTreeMap<Instant, Vec<SharedRef<Timer>>>> =
    SpinLock::new(BTreeMap::new());

enum State {
    /// The timer has beeen created but not set yet, or it has been fired
    /// and removed from `TIMERS`.
    InActive,
    /// The timer is active, that is, it is waiting for the deadline to
    /// be reached.
    Active { deadline: Instant },
}

pub struct Timer {
    mutable: SpinLock<State>,
    emitter: PollEmitter,
}

impl Timer {
    pub fn new() -> Self {
        Self {
            mutable: SpinLock::new(State::InActive),
            emitter: PollEmitter::new(Readiness::empty()),
        }
    }

    pub fn set(self: SharedRef<Self>, deadline: Instant) {
        let mut timers = TIMERS.lock();
        let mut state = self.mutable.lock();

        // Remove the timer from TIMERS if it's active.
        if let State::Active { deadline } = &*state {
            // SAFETY: The timer should be in TIMERS if deadline is set.
            let items = timers.get_mut(deadline).unwrap();

            items.retain(|item| !SharedRef::ptr_eq(item, &self));
            if items.is_empty() {
                timers.remove(deadline);
            }
        }

        // Add the timer with the new deadline to TIMERS.
        *state = State::Active { deadline };

        // The timer is now reset.
        self.emitter.clear(Readiness::READABLE);

        drop(state);
        timers.entry(deadline).or_default().push(self);

        arch::set_timer(deadline);
    }

    pub fn fire(self: &SharedRef<Self>) {
        let mut state = self.mutable.lock();
        debug_assert!(matches!(&*state, State::Active { .. }));

        *state = State::InActive;
        self.emitter.set(Readiness::READABLE);
    }
}

impl Handleable for Timer {
    fn emitter(&self) -> Option<&PollEmitter> {
        Some(&self.emitter)
    }

    fn is_edge_triggered(&self) -> bool {
        true
    }
}

/// Checks for expired timers and fires them.
///
/// Returns the deadline of the most imminent timer if there is one.
fn fire_expired_timers() -> Option<Instant> {
    let mut timers = TIMERS.lock();
    let now = arch::read_monotonic_time();
    while let Some((deadline, _)) = timers.first_key_value() {
        if *deadline > now {
            // This item is not yet expired. We can ignore the rest
            // of the timers because they're sorted by deadline.
            return Some(*deadline);
        }

        // Remove the expired timers and notify the listeners.
        let expired_timers = timers.pop_first().unwrap().1;
        for timer in expired_timers {
            timer.fire();
        }
    }

    None
}

pub fn handle_timer_interrupt() {
    trace!("handling timer interrupt");
    if let Some(next_deadline) = fire_expired_timers() {
        arch::set_timer(next_deadline);
    }
}

pub fn sys_timer_now() -> Result<SyscallResult, ErrorCode> {
    let instant = arch::read_monotonic_time();

    // TODO: 32-bit systems need something better - 2^32 nanoseconds = 4 seconds,
    //       which is way too short.
    static_assert!(size_of::<u64>() == size_of::<usize>());
    Ok(SyscallResult::Return(instant.as_raw() as usize))
}

pub fn sys_timer_create(current: &SharedRef<Thread>) -> Result<SyscallResult, ErrorCode> {
    let timer = SharedRef::new(Timer::new())?;
    let handle = Handle::new(timer, HandleRight::READ | HandleRight::WRITE);

    let handle_id = current.process().handles().lock().insert(handle)?;

    Ok(SyscallResult::Return(handle_id.as_usize()))
}

pub fn sys_timer_set(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
) -> Result<SyscallResult, ErrorCode> {
    let handle_id = HandleId::from_usize(a0)?;
    let deadline = Instant::from_usize(a1)?;

    let handle_table = current.process().handles().lock();
    let timer = handle_table
        .get::<Timer>(handle_id)?
        .authorize(HandleRight::WRITE)?;

    timer.set(deadline);

    Ok(SyscallResult::Return(0))
}
