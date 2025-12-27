use ftl_types::handle::HandleId;

use crate::handle::Handleable;
use crate::handle::OwnedHandle;

pub(crate) static CURRENT_PROCESS: Process =
    Process::from_handle(OwnedHandle::from_raw(HandleId::from_raw(2)));

pub struct Process {
    handle: OwnedHandle,
}

impl Process {
    pub const fn from_handle(handle: OwnedHandle) -> Self {
        Self { handle }
    }
}

impl Handleable for Process {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}
