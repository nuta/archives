use core::arch::naked_asm;
use core::mem::offset_of;

use super::Thread;
use crate::arch::arm64::boot::SP_BOTTOM_SHIFT;
use crate::cpuvar::CpuVar;
use crate::syscall::handle_syscall;

/// The entry point for system calls from in-kernel apps.
#[unsafe(naked)]
pub unsafe extern "C" fn kernelcall_handler() -> ! {
    // Save callee-saved registers. Caller-saved registers are
    // saved on the stack by the caller.
    //
    // Registers:
    //
    // - x0:      syscall number
    // - x1-x6:   syscall arguments
    // - x10:     *mut Thread
    // - x11:     temporary values
    // - x19-x29: callee-saved registers (to be saved)
    // - sp:      user's stack pointer (to be saved)
    // - lr:      return address (to be saved)
    naked_asm!(
        // Get the kernel stack pointer.
        "mrs x10, tpidr_el1",

        // Calculate the CPU variable address.
        "lsr x10, x10, {sp_bottom_shift}",
        "lsl x10, x10, {sp_bottom_shift}",

        // Load the pointer to the current thread.
        "ldr x10, [x10, {current_thread_offset}]",

        // Save callee-saved registers.
        "str x19, [x10, {x19_offset}]",
        "str x20, [x10, {x20_offset}]",
        "str x21, [x10, {x21_offset}]",
        "str x22, [x10, {x22_offset}]",
        "str x23, [x10, {x23_offset}]",
        "str x24, [x10, {x24_offset}]",
        "str x25, [x10, {x25_offset}]",
        "str x26, [x10, {x26_offset}]",
        "str x27, [x10, {x27_offset}]",
        "str x28, [x10, {x28_offset}]",
        "str x29, [x10, {x29_offset}]",

        // Save user's stack pointer.
        "mov x11, sp",
        "str x11, [x10, {sp_offset}]",

        // Save the return address.
        "str lr, [x10, {elr_offset}]",

        // Save tpidr_el0.
        "mrs x11, tpidr_el0",
        "str x11, [x10, {tpidr_el0_offset}]",

        // Switch to the kernel stack.
        "mrs x11, tpidr_el1",
        "msr spsel, #1",
        "mov sp, x11",

        "b {handle_syscall}",
        current_thread_offset = const offset_of!(CpuVar, current_thread),
        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        x19_offset = const offset_of!(Thread, x19),
        x20_offset = const offset_of!(Thread, x20),
        x21_offset = const offset_of!(Thread, x21),
        x22_offset = const offset_of!(Thread, x22),
        x23_offset = const offset_of!(Thread, x23),
        x24_offset = const offset_of!(Thread, x24),
        x25_offset = const offset_of!(Thread, x25),
        x26_offset = const offset_of!(Thread, x26),
        x27_offset = const offset_of!(Thread, x27),
        x28_offset = const offset_of!(Thread, x28),
        x29_offset = const offset_of!(Thread, x29),
        sp_offset = const offset_of!(Thread, sp),
        elr_offset = const offset_of!(Thread, elr),
        tpidr_el0_offset = const offset_of!(Thread, tpidr_el0),
        handle_syscall = sym handle_syscall,
    );
}
