use ftl_types::error::ErrorCode;
pub use ftl_types::handle::HandleId;
use log::warn;

#[derive(Debug)]
pub struct OwnedHandle(HandleId);

impl OwnedHandle {
    pub const fn from_raw(raw: HandleId) -> Self {
        Self(raw)
    }

    pub fn id(&self) -> HandleId {
        self.0
    }
}

impl Drop for OwnedHandle {
    fn drop(&mut self) {
        // log::trace!("dropping handle {:?}", self.0);
        if let Err(e) = sys_handle_close(self.0) {
            warn!("failed to close handle {:?}: {:?}", self.0, e);
        }
    }
}

pub trait Handleable {
    fn handle_id(&self) -> HandleId;
}

fn sys_handle_close(handle_id: HandleId) -> Result<(), ErrorCode> {
    use ftl_types::syscall::SYS_HANDLE_CLOSE;

    use crate::syscall::syscall1;

    syscall1(SYS_HANDLE_CLOSE, handle_id.as_usize())?;
    Ok(())
}
