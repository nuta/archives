use core::arch::naked_asm;
use core::mem::offset_of;

use super::Thread;
use super::boot::SP_BOTTOM_SHIFT;
use crate::cpuvar::CpuVar;

/// The entry point for system calls from in-kernel apps.
#[unsafe(naked)]
pub unsafe extern "C" fn kernelcall_handler() -> ! {
    naked_asm!(
        // Save callee-saved registers. Caller-saved registers are
        // saved on the stack by the caller.
        "csrrw sp, sscratch, sp",
        "srli t0, sp, {sp_bottom_shift}",
        "slli t0, t0, {sp_bottom_shift}",

        "ld t0, {current_thread_offset}(t0)",
        "sd tp, {tp_offset}(t0)",
        "sd gp, {gp_offset}(t0)",
        "sd s0, {s0_offset}(t0)",
        "sd s1, {s1_offset}(t0)",
        "sd s2, {s2_offset}(t0)",
        "sd s3, {s3_offset}(t0)",
        "sd s4, {s4_offset}(t0)",
        "sd s5, {s5_offset}(t0)",
        "sd s6, {s6_offset}(t0)",
        "sd s7, {s7_offset}(t0)",
        "sd s8, {s8_offset}(t0)",
        "sd s9, {s9_offset}(t0)",
        "sd s10, {s10_offset}(t0)",
        "sd s11, {s11_offset}(t0)",
        "sd ra, {sepc_offset}(t0)",

        "csrrw t1, sscratch, sp",
        "sd t1, {sp_offset}(t0)",

        "j {handle_syscall}",
        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        current_thread_offset = const offset_of!(CpuVar, current_thread),
        sp_offset = const offset_of!(Thread, sp),
        gp_offset = const offset_of!(Thread, gp),
        tp_offset = const offset_of!(Thread, tp),
        s0_offset = const offset_of!(Thread, s0),
        s1_offset = const offset_of!(Thread, s1),
        s2_offset = const offset_of!(Thread, s2),
        s3_offset = const offset_of!(Thread, s3),
        s4_offset = const offset_of!(Thread, s4),
        s5_offset = const offset_of!(Thread, s5),
        s6_offset = const offset_of!(Thread, s6),
        s7_offset = const offset_of!(Thread, s7),
        s8_offset = const offset_of!(Thread, s8),
        s9_offset = const offset_of!(Thread, s9),
        s10_offset = const offset_of!(Thread, s10),
        s11_offset = const offset_of!(Thread, s11),
        sepc_offset = const offset_of!(Thread, sepc),
        handle_syscall = sym crate::syscall::handle_syscall,
    );
}
