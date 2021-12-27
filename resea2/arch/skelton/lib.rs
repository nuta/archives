use arch_common::{paddr::PAddr, vaddr::VAddr};

pub const PAGE_SIZE: usize = 4096;

#[no_mangle]
pub extern "C" fn printchar(c: u8) {
    // For debugging in unit testing.
    unsafe {
        libc::putchar(c.into());
    }
}

pub trait TryAsVAddr {
    fn try_as_vaddr(&self) -> Option<VAddr>;
}

impl TryAsVAddr for PAddr {
    fn try_as_vaddr(&self) -> Option<VAddr> {
        todo!()
    }
}

pub struct ArchThread {}

impl ArchThread {
    pub fn new_kernel_thread(_entry: VAddr, _stack_top: VAddr) -> ArchThread {
        todo!()
    }

    pub fn new_idle_thread() -> ArchThread {
        todo!()
    }
}

pub fn switch_thread(_prev: &ArchThread, _next: &ArchThread) {
    todo!();
}
