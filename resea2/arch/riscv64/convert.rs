use arch_common::{paddr::PAddr, vaddr::VAddr};

pub trait TryAsVAddr {
    fn try_as_vaddr(&self) -> Option<VAddr>;
}

impl TryAsVAddr for PAddr {
    fn try_as_vaddr(&self) -> Option<VAddr> {
        // SAFETY: The physical memory addresses are straighted mapped in the
        //         virtual memory space.
        Some(unsafe { VAddr::new(self.as_usize()) })
    }
}
