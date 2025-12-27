use core::arch::asm;
use core::arch::naked_asm;
use core::mem::offset_of;

use super::Thread;
use super::boot::SP_BOTTOM_SHIFT;
use crate::address::UAddr;
use crate::arch::INTERRUPT_CONTROLLER;
use crate::arch::get_cpuvar;
use crate::cpuvar::CpuVar;
use crate::thread::return_to_user;

/// Note: Do not save any state here; this will also be called from the idle
/// context, and in that case the user registers will be overwritten.
pub fn handle_trap() -> ! {
    let scause: u64;
    let stval: u64;
    unsafe {
        asm!("csrr {0}, scause", out(reg) scause);
        asm!("csrr {0}, stval", out(reg) stval);
    }

    let is_intr = scause & (1 << 63) != 0;
    let code = scause & !(1 << 63);
    let scause_str = match (is_intr, code) {
        (true, 0) => "user software interrupt",
        (true, 1) => "supervisor software interrupt",
        (true, 2) => "hypervisor software interrupt",
        (true, 3) => "machine software interrupt",
        (true, 4) => "user timer interrupt",
        (true, 5) => "supervisor timer interrupt",
        (true, 6) => "hypervisor timer interrupt",
        (true, 7) => "machine timer interrupt",
        (true, 8) => "user external interrupt",
        (true, 9) => "supervisor external interrupt",
        (true, 10) => "hypervisor external interrupt",
        (true, 11) => "machine external interrupt",
        (false, 0) => "instruction address misaligned",
        (false, 1) => "instruction access fault",
        (false, 2) => "illegal instruction",
        (false, 3) => "breakpoint",
        (false, 4) => "load address misaligned",
        (false, 5) => "load access fault",
        (false, 6) => "store/AMO address misaligned",
        (false, 7) => "store/AMO access fault",
        (false, 8) => "environment call from U-mode",
        (false, 9) => "environment call from S-mode",
        (false, 10) => "Environment call from VS-mode",
        (false, 11) => "environment call from M-mode",
        (false, 12) => "instruction page fault",
        (false, 13) => "load page fault",
        (false, 15) => "store/AMO page fault",
        (false, 20) => "instruction guest-page fault",
        (false, 21) => "load guest-page fault",
        (false, 22) => "virtual instruction",
        (false, 23) => "store/AMO guest-page fault",
        _ => "unknown",
    };

    info!("trap: {scause_str}, stval: {stval:x}");

    match (is_intr, code) {
        (false, 13) | (false, 15) => {
            // Load page fault (13) or store/AMO page fault (15)
            // TODO: check if the operation is allowed in the mapping
            if let Err(err) = get_cpuvar()
                .current_thread
                .process()
                .isolation()
                .vmspace()
                .handle_page_fault(UAddr::new(stval as usize))
            {
                panic!("failed to handle page fault: {err:?}"); // TODO:
            }
        }
        (true, 9) => {
            if let Some(irq) = INTERRUPT_CONTROLLER.get_pending_irq() {
                crate::interrupt::handle_irq(irq);
            }
        }
        (true, 5) => {
            super::timer::acknowledge_timer();
            crate::timer::handle_timer_interrupt();
        }
        _ => {
            panic!("unhandled trap: {scause_str}, stval: {stval:x}");
        }
    }

    return_to_user();
}

/// The entry point for traps: exceptions, interrupts, and system calls.
#[unsafe(naked)]
#[rustc_align(4)] // Avoid overlapping with lower bits of stvec.
pub unsafe extern "C" fn trap_handler() -> ! {
    naked_asm!(
        "csrrw sp, sscratch, sp",      // Save sp to sscratch, and load the kernel stack
        "sd a0, -8(sp)",               // Save a0 temporarily onto the stack

        // Calculate the CPU variable address.
        "srli a0, sp, {sp_bottom_shift}",
        "slli a0, a0, {sp_bottom_shift}",

        // Load the pointer to the current thread.
        "ld a0, {current_thread_offset}(a0)",

        // Save user's general-purpose registers.
        "sd a1, {a1_offset}(a0)",
        "sd a2, {a2_offset}(a0)",
        "sd a3, {a3_offset}(a0)",
        "sd a4, {a4_offset}(a0)",
        "sd a5, {a5_offset}(a0)",
        "sd a6, {a6_offset}(a0)",
        "sd a7, {a7_offset}(a0)",
        "sd t0, {t0_offset}(a0)",
        "sd t1, {t1_offset}(a0)",
        "sd t2, {t2_offset}(a0)",
        "sd t3, {t3_offset}(a0)",
        "sd t4, {t4_offset}(a0)",
        "sd t5, {t5_offset}(a0)",
        "sd t6, {t6_offset}(a0)",
        "sd s0, {s0_offset}(a0)",
        "sd s1, {s1_offset}(a0)",
        "sd s2, {s2_offset}(a0)",
        "sd s3, {s3_offset}(a0)",
        "sd s4, {s4_offset}(a0)",
        "sd s5, {s5_offset}(a0)",
        "sd s6, {s6_offset}(a0)",
        "sd s7, {s7_offset}(a0)",
        "sd s8, {s8_offset}(a0)",
        "sd s9, {s9_offset}(a0)",
        "sd s10, {s10_offset}(a0)",
        "sd s11, {s11_offset}(a0)",
        "sd ra, {ra_offset}(a0)",
        "sd tp, {tp_offset}(a0)",
        "sd gp, {gp_offset}(a0)",

        "csrr t0, sepc",
        "sd t0, {sepc_offset}(a0)",

        // Restore a0 and save it into the context.
        "ld t0, -8(sp)",
        "sd t0, {a0_offset}(a0)",

        // Restore the user's sp, and restore kernel sp into sscratch.
        "csrrw t0, sscratch, sp",
        "sd t0, {sp_offset}(a0)",

        "j {handle_trap}",
        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        current_thread_offset = const offset_of!(CpuVar, current_thread),
        sepc_offset = const offset_of!(Thread, sepc),
        ra_offset = const offset_of!(Thread, ra),
        sp_offset = const offset_of!(Thread, sp),
        gp_offset = const offset_of!(Thread, gp),
        tp_offset = const offset_of!(Thread, tp),
        t0_offset = const offset_of!(Thread, t0),
        t1_offset = const offset_of!(Thread, t1),
        t2_offset = const offset_of!(Thread, t2),
        s0_offset = const offset_of!(Thread, s0),
        s1_offset = const offset_of!(Thread, s1),
        a0_offset = const offset_of!(Thread, a0),
        a1_offset = const offset_of!(Thread, a1),
        a2_offset = const offset_of!(Thread, a2),
        a3_offset = const offset_of!(Thread, a3),
        a4_offset = const offset_of!(Thread, a4),
        a5_offset = const offset_of!(Thread, a5),
        a6_offset = const offset_of!(Thread, a6),
        a7_offset = const offset_of!(Thread, a7),
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
        t3_offset = const offset_of!(Thread, t3),
        t4_offset = const offset_of!(Thread, t4),
        t5_offset = const offset_of!(Thread, t5),
        t6_offset = const offset_of!(Thread, t6),
        handle_trap = sym handle_trap,
    )
}
