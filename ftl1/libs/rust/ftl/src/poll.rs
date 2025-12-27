use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::poll::PollEvent;
pub use ftl_types::poll::Readiness;
use ftl_types::syscall::SYS_POLL_ADD;
use ftl_types::syscall::SYS_POLL_CREATE;
use ftl_types::syscall::SYS_POLL_REMOVE;
use ftl_types::syscall::SYS_POLL_UPDATE;
use ftl_types::syscall::SYS_POLL_WAIT;

use crate::handle::OwnedHandle;
use crate::syscall::syscall0;
use crate::syscall::syscall2;
use crate::syscall::syscall3;
use crate::syscall::syscall4;

pub struct Poll {
    handle: OwnedHandle,
}

impl Poll {
    pub fn new() -> Result<Self, ErrorCode> {
        let ret = syscall0(SYS_POLL_CREATE)?;
        let handle_id = HandleId::from_usize(ret)?;
        let handle = OwnedHandle::from_raw(handle_id);
        Ok(Self { handle })
    }

    pub fn add(&self, listenee: HandleId, interests: Readiness) -> Result<(), ErrorCode> {
        syscall3(
            SYS_POLL_ADD,
            self.handle.id().as_usize(),
            listenee.as_usize(),
            interests.as_usize(),
        )?;
        Ok(())
    }

    pub fn update(
        &self,
        listenee: HandleId,
        or_mask: Readiness,
        and_mask: Readiness,
    ) -> Result<(), ErrorCode> {
        syscall4(
            SYS_POLL_UPDATE,
            self.handle.id().as_usize(),
            listenee.as_usize(),
            or_mask.as_usize(),
            and_mask.as_usize(),
        )?;
        Ok(())
    }

    pub fn wait(&self) -> Result<(HandleId, Readiness), ErrorCode> {
        let ret = syscall4(SYS_POLL_WAIT, self.handle.id().as_usize(), 0, 0, 0)?;
        let event = PollEvent::from_raw(ret)?;
        Ok((event.handle_id, event.readiness))
    }

    pub fn remove(&self, listenee: HandleId) -> Result<(), ErrorCode> {
        syscall2(
            SYS_POLL_REMOVE,
            self.handle.id().as_usize(),
            listenee.as_usize(),
        )?;

        Ok(())
    }
}
