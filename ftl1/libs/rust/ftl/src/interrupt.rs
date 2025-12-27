pub use ftl_types::device::IrqDesc;
use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::syscall::SYS_INTERRUPT_ACKNOWLEDGE;
use ftl_types::syscall::SYS_INTERRUPT_ACQUIRE;

use crate::handle::Handleable;
use crate::handle::OwnedHandle;
use crate::syscall::syscall1;

#[derive(Debug)]
pub struct Interrupt {
    handle: OwnedHandle,
}

impl Interrupt {
    pub fn acquire(desc: IrqDesc) -> Result<Self, ErrorCode> {
        let handle_id = syscall1(SYS_INTERRUPT_ACQUIRE, desc.as_raw())?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(handle_id as i32));
        Ok(Self { handle })
    }

    pub fn acknowledge(&self) -> Result<(), ErrorCode> {
        syscall1(SYS_INTERRUPT_ACKNOWLEDGE, self.handle.id().as_usize())?;
        Ok(())
    }
}

impl Handleable for Interrupt {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}
