use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::syscall::SYS_TIMER_CREATE;
use ftl_types::syscall::SYS_TIMER_NOW;
use ftl_types::syscall::SYS_TIMER_SET;
pub use ftl_types::timer::Duration;
pub use ftl_types::timer::Instant;
use ftl_utils::static_assert;

use crate::handle::Handleable;
use crate::handle::OwnedHandle;
use crate::syscall::syscall0;
use crate::syscall::syscall2;

pub fn now() -> Instant {
    // SAFETY: timer_now syscall should never fail.
    let raw = syscall0(SYS_TIMER_NOW).expect("timer_now syscall failed");
    Instant::from_raw(raw as u64)
}

#[derive(Debug)]
pub struct Timer {
    handle: OwnedHandle,
}

impl Timer {
    pub fn new() -> Result<Self, ErrorCode> {
        let ret = syscall0(SYS_TIMER_CREATE)?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(ret as i32));
        Ok(Self { handle })
    }

    pub fn set(&self, deadline: Instant) -> Result<(), ErrorCode> {
        // TODO: We need to use 2 registers to pass the deadline on
        //       32-bit systems.
        static_assert!(
            !cfg!(target_pointer_width = "32"),
            "timer set is not supported on 32-bit systems"
        );

        syscall2(
            SYS_TIMER_SET,
            self.handle.id().as_usize(),
            deadline.as_raw().try_into().unwrap(),
        )?;
        Ok(())
    }
}

impl Handleable for Timer {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}
