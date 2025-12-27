/// Read barrier.
pub(crate) fn read_barrier() {
    #[cfg(target_arch = "aarch64")]
    unsafe {
        core::arch::asm!("dmb oshld");
    }
}

/// Write barrier.
pub(crate) fn write_barrier() {
    #[cfg(target_arch = "aarch64")]
    unsafe {
        core::arch::asm!("dmb oshst");
    }
}
