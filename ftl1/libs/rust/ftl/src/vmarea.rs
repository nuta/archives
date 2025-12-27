use ftl_types::error::ErrorCode;
use ftl_types::syscall::SYS_VMAREA_ALLOC;
use ftl_types::syscall::SYS_VMAREA_ALLOC_CONTIGUOUS;
use ftl_types::syscall::SYS_VMAREA_PIN;

use crate::handle::HandleId;
use crate::handle::Handleable;
use crate::handle::OwnedHandle;
use crate::syscall::syscall1;
use crate::syscall::syscall2;

pub struct VmArea {
    handle: OwnedHandle,
}

impl VmArea {
    /// Allocates arbitrary pages.
    pub fn alloc(size: usize) -> Result<Self, ErrorCode> {
        let handle_id = syscall1(SYS_VMAREA_ALLOC, size)?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(handle_id as i32));
        Ok(Self { handle })
    }

    /// Allocates at the given physical address.
    pub fn new_pinned(paddr: usize, size: usize) -> Result<Self, ErrorCode> {
        let handle_id = syscall2(SYS_VMAREA_PIN, paddr, size)?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(handle_id as i32));
        Ok(Self { handle })
    }

    /// Allocates arbitrary physically-contiguous pages.
    pub fn alloc_contiguous(size: usize) -> Result<(Self, usize), ErrorCode> {
        let mut paddr: usize = 0;
        let handle_id = syscall2(
            SYS_VMAREA_ALLOC_CONTIGUOUS,
            size,
            &mut paddr as *mut usize as usize,
        )?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(handle_id as i32));
        Ok((Self { handle }, paddr))
    }

    pub fn handle(&self) -> &OwnedHandle {
        &self.handle
    }
}

impl Handleable for VmArea {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}
