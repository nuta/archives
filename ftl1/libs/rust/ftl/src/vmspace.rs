use ftl_types::arch::PageAttrs;
use ftl_types::error::ErrorCode;
use ftl_types::syscall::SYS_VMSPACE_MAP;

use crate::handle::HandleId;
use crate::handle::Handleable;
use crate::handle::OwnedHandle;
use crate::syscall::syscall6;
use crate::vmarea::VmArea;

pub static CURRENT_VMSPACE: VmSpace =
    VmSpace::from_handle(OwnedHandle::from_raw(HandleId::from_raw(1)));

pub struct VmSpace {
    handle: OwnedHandle,
}

impl VmSpace {
    pub const fn from_handle(handle: OwnedHandle) -> Self {
        Self { handle }
    }

    /// Map a VmArea into this VmSpace.
    ///
    /// If `vaddr` is 0, the kernel will choose an address.
    /// Returns the base address of the mapping.
    pub fn map(
        &self,
        vmarea: &VmArea,
        vaddr: usize,
        offset: usize,
        len: usize,
        attrs: PageAttrs,
    ) -> Result<usize, ErrorCode> {
        syscall6(
            SYS_VMSPACE_MAP,
            self.handle.id().as_usize(),
            vmarea.handle_id().as_usize(),
            vaddr,
            offset,
            len,
            attrs.to_usize(),
        )
    }

    pub fn handle(&self) -> &OwnedHandle {
        &self.handle
    }
}

impl Handleable for VmSpace {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}
