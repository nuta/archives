#[inline(always)]
pub unsafe fn mret() {
    #[cfg(target_arch = "riscv64")]
    asm!("mret");
}
