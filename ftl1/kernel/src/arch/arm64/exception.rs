use core::arch::asm;
use core::arch::global_asm;
use core::arch::naked_asm;
use core::mem::offset_of;

use crate::address::UAddr;
use crate::arch::INTERRUPT_CONTROLLER;
use crate::arch::Thread;
use crate::arch::arm64::boot::SP_BOTTOM_SHIFT;
use crate::arch::get_cpuvar;
use crate::cpuvar::CpuVar;
use crate::thread::return_to_user;

unsafe extern "C" {
    pub static default_vector_table: u8;
    pub static idle_vector_table: u8;
}

global_asm!(
    r#"
.balign 2048
.globl default_vector_table
default_vector_table:
    // EL1t
    b exception_handler;   .balign 128
    b interrupt_handler;   .balign 128
    b unreachable_handler; .balign 128
    b unreachable_handler; .balign 128

    // EL1h
    b exception_handler;   .balign 128
    b interrupt_handler;   .balign 128
    b unreachable_handler; .balign 128
    b unreachable_handler; .balign 128

    // EL0 (64-bit)
    b exception_handler;   .balign 128
    b interrupt_handler;   .balign 128
    b unreachable_handler; .balign 128
    b unreachable_handler; .balign 128

    // EL0 (32-bit)
    b unreachable_handler; .balign 128
    b unreachable_handler; .balign 128
    b unreachable_handler; .balign 128
    b unreachable_handler; .balign 128
"#,
);

global_asm!(
    r#"
.balign 2048
.globl idle_vector_table
idle_vector_table:
    // EL1t
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128

    // EL1h
    b idle_unreachable_handler; .balign 128
    b idle_interrupt_handler;   .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128

    // EL0 (64-bit)
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128

    // EL0 (32-bit)
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
    b idle_unreachable_handler; .balign 128
"#,
);

#[unsafe(no_mangle)]
fn idle_interrupt_handler() -> ! {
    unsafe {
        asm!("msr daifset, #2");
        asm!("msr vbar_el1, {}", in(reg) &raw const default_vector_table as usize);
    }
    handle_interrupt();
}

#[unsafe(no_mangle)]
fn idle_unreachable_handler() -> ! {
    unreachable!("unexpected exception (in idle)");
}

pub fn handle_exception() -> ! {
    let esr: u64;
    let far: u64;
    let elr: u64;
    unsafe {
        asm!("mrs {0}, esr_el1", out(reg) esr);
        asm!("mrs {0}, far_el1", out(reg) far);
        asm!("mrs {0}, elr_el1", out(reg) elr);
    }

    let ec = (esr >> 26) & 0x3f;
    let _iss = esr & 0x1ffffff;
    let ec_str = match ec {
        0x00 => "unknown",
        0x01 => "trapped WFI/WFE",
        0x03 => "trapped MCR/MRC (CP15)",
        0x04 => "trapped MCRR/MRRC (CP15)",
        0x05 => "trapped MCR/MRC (CP14)",
        0x06 => "trapped LDC/STC",
        0x07 => "trapped SVE/SIMD/FP",
        0x0c => "trapped MRRC (CP14)",
        0x0e => "illegal execution state",
        0x11 => "SVC instruction from AArch32",
        0x12 => "HVC instruction from AArch32",
        0x13 => "SMC instruction from AArch32",
        0x15 => "SVC instruction from AArch64",
        0x16 => "HVC instruction from AArch64",
        0x17 => "SMC instruction from AArch64",
        0x18 => "trapped MRS/MSR (system register)",
        0x19 => "trapped SVE",
        0x1f => "implementation defined (EL3)",
        0x20 => "instruction abort from lower EL",
        0x21 => "instruction abort from same EL",
        0x22 => "PC alignment fault",
        0x24 => "data abort from lower EL",
        0x25 => "data abort from same EL",
        0x26 => "SP alignment fault",
        0x28 => "trapped floating-point (AArch32)",
        0x2c => "trapped floating-point (AArch64)",
        0x2f => "SError interrupt",
        0x30 => "breakpoint from lower EL",
        0x31 => "breakpoint from same EL",
        0x32 => "software step from lower EL",
        0x33 => "software step from same EL",
        0x34 => "watchpoint from lower EL",
        0x35 => "watchpoint from same EL",
        0x38 => "BKPT instruction (AArch32)",
        0x3c => "BRK instruction (AArch64)",
        _ => "reserved",
    };

    match ec {
        0x24 | 0x25 => {
            if let Err(err) = get_cpuvar()
                .current_thread
                .process()
                .isolation()
                .vmspace()
                .handle_page_fault(UAddr::new(far as usize))
            {
                panic!("failed to handle page fault: {err:?}, far: {far:x}, elr: {elr:x}");
            }
        }
        _ => {
            panic!("unhandled exception: {ec_str}, far: {far:x}");
        }
    }

    return_to_user();
}

#[unsafe(naked)]
#[unsafe(no_mangle)]
pub extern "C" fn exception_handler() -> ! {
    // SP is already switched to EL1h (i.e. per-CPU kernel stack).
    naked_asm!(
        "str x0, [sp, -8]", // Save x0 to the kernel stack temporarily

        // Calculate the CPU variable address.
        "mov x0, sp",
        "lsr x0, x0, {sp_bottom_shift}",
        "lsl x0, x0, {sp_bottom_shift}",

        // Load the pointer to the current thread.
        "ldr x0, [x0, {current_thread_offset}]",

        // Save user's general-purpose registers.
        "str x1, [x0, {x1_offset}]",
        "str x2, [x0, {x2_offset}]",
        "str x3, [x0, {x3_offset}]",
        "str x4, [x0, {x4_offset}]",
        "str x5, [x0, {x5_offset}]",
        "str x6, [x0, {x6_offset}]",
        "str x7, [x0, {x7_offset}]",
        "str x8, [x0, {x8_offset}]",
        "str x9, [x0, {x9_offset}]",
        "str x10, [x0, {x10_offset}]",
        "str x11, [x0, {x11_offset}]",
        "str x12, [x0, {x12_offset}]",
        "str x13, [x0, {x13_offset}]",
        "str x14, [x0, {x14_offset}]",
        "str x15, [x0, {x15_offset}]",
        "str x16, [x0, {x16_offset}]",
        "str x17, [x0, {x17_offset}]",
        "str x18, [x0, {x18_offset}]",
        "str x19, [x0, {x19_offset}]",
        "str x20, [x0, {x20_offset}]",
        "str x21, [x0, {x21_offset}]",
        "str x22, [x0, {x22_offset}]",
        "str x23, [x0, {x23_offset}]",
        "str x24, [x0, {x24_offset}]",
        "str x25, [x0, {x25_offset}]",
        "str x26, [x0, {x26_offset}]",
        "str x27, [x0, {x27_offset}]",
        "str x28, [x0, {x28_offset}]",
        "str x29, [x0, {x29_offset}]",
        "str x30, [x0, {x30_offset}]",

        // Save user's program counter.
        "mrs x1, elr_el1",
        "str x1, [x0, {elr_offset}]",

        // Save processor state.
        "mrs x1, spsr_el1",
        "str x1, [x0, {spsr_offset}]",

        // Save user's stack pointer.
        "mrs x1, sp_el0",
        "str x1, [x0, {sp_offset}]",

        // Save tpidr_el0.
        "mrs x1, tpidr_el0",
        "str x1, [x0, {tpidr_el0_offset}]",

        // Restore x0 from the kernel stack.
        "ldr x1, [sp, -8]",
        "str x1, [x0, {x0_offset}]",

        "b {handle_exception}",

        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        current_thread_offset = const offset_of!(CpuVar, current_thread),
        spsr_offset = const offset_of!(Thread, spsr),
        sp_offset = const offset_of!(Thread, sp),
        tpidr_el0_offset = const offset_of!(Thread, tpidr_el0),
        x0_offset = const offset_of!(Thread, x0),
        x1_offset = const offset_of!(Thread, x1),
        x2_offset = const offset_of!(Thread, x2),
        x3_offset = const offset_of!(Thread, x3),
        x4_offset = const offset_of!(Thread, x4),
        x5_offset = const offset_of!(Thread, x5),
        x6_offset = const offset_of!(Thread, x6),
        x7_offset = const offset_of!(Thread, x7),
        x8_offset = const offset_of!(Thread, x8),
        x9_offset = const offset_of!(Thread, x9),
        x10_offset = const offset_of!(Thread, x10),
        x11_offset = const offset_of!(Thread, x11),
        x12_offset = const offset_of!(Thread, x12),
        x13_offset = const offset_of!(Thread, x13),
        x14_offset = const offset_of!(Thread, x14),
        x15_offset = const offset_of!(Thread, x15),
        x16_offset = const offset_of!(Thread, x16),
        x17_offset = const offset_of!(Thread, x17),
        x18_offset = const offset_of!(Thread, x18),
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
        x30_offset = const offset_of!(Thread, x30),
        elr_offset = const offset_of!(Thread, elr),
        handle_exception = sym handle_exception,
    );
}

pub fn handle_interrupt() -> ! {
    if let Some(irq) = INTERRUPT_CONTROLLER.get_pending_irq() {
        if irq == 27 {
            super::acknowledge_timer();
            crate::timer::handle_timer_interrupt();
        } else {
            crate::interrupt::handle_irq(irq);
        }
        INTERRUPT_CONTROLLER.acknowledge(irq);
    }

    return_to_user();
}

#[unsafe(naked)]
#[unsafe(no_mangle)]
pub extern "C" fn interrupt_handler() -> ! {
    naked_asm!(
        "str x0, [sp, -8]", // Save x0 to the kernel stack temporarily

        // Calculate the CPU variable address.
        "mov x0, sp",
        "lsr x0, x0, {sp_bottom_shift}",
        "lsl x0, x0, {sp_bottom_shift}",

        // Load the pointer to the current thread.
        "ldr x0, [x0, {current_thread_offset}]",

        // Save user's general-purpose registers.
        "str x1, [x0, {x1_offset}]",
        "str x2, [x0, {x2_offset}]",
        "str x3, [x0, {x3_offset}]",
        "str x4, [x0, {x4_offset}]",
        "str x5, [x0, {x5_offset}]",
        "str x6, [x0, {x6_offset}]",
        "str x7, [x0, {x7_offset}]",
        "str x8, [x0, {x8_offset}]",
        "str x9, [x0, {x9_offset}]",
        "str x10, [x0, {x10_offset}]",
        "str x11, [x0, {x11_offset}]",
        "str x12, [x0, {x12_offset}]",
        "str x13, [x0, {x13_offset}]",
        "str x14, [x0, {x14_offset}]",
        "str x15, [x0, {x15_offset}]",
        "str x16, [x0, {x16_offset}]",
        "str x17, [x0, {x17_offset}]",
        "str x18, [x0, {x18_offset}]",
        "str x19, [x0, {x19_offset}]",
        "str x20, [x0, {x20_offset}]",
        "str x21, [x0, {x21_offset}]",
        "str x22, [x0, {x22_offset}]",
        "str x23, [x0, {x23_offset}]",
        "str x24, [x0, {x24_offset}]",
        "str x25, [x0, {x25_offset}]",
        "str x26, [x0, {x26_offset}]",
        "str x27, [x0, {x27_offset}]",
        "str x28, [x0, {x28_offset}]",
        "str x29, [x0, {x29_offset}]",
        "str x30, [x0, {x30_offset}]",

        // Save user's program counter.
        "mrs x1, elr_el1",
        "str x1, [x0, {elr_offset}]",

        // Save processor state.
        "mrs x1, spsr_el1",
        "str x1, [x0, {spsr_offset}]",

        // Save user's stack pointer.
        "mrs x1, sp_el0",
        "str x1, [x0, {sp_offset}]",

        // Save tpidr_el0.
        "mrs x1, tpidr_el0",
        "str x1, [x0, {tpidr_el0_offset}]",

        // Restore x0 from the kernel stack.
        "ldr x1, [sp, -8]",
        "str x1, [x0, {x0_offset}]",

        "b {handle_interrupt}",
        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        current_thread_offset = const offset_of!(CpuVar, current_thread),
        spsr_offset = const offset_of!(Thread, spsr),
        sp_offset = const offset_of!(Thread, sp),
        tpidr_el0_offset = const offset_of!(Thread, tpidr_el0),
        x0_offset = const offset_of!(Thread, x0),
        x1_offset = const offset_of!(Thread, x1),
        x2_offset = const offset_of!(Thread, x2),
        x3_offset = const offset_of!(Thread, x3),
        x4_offset = const offset_of!(Thread, x4),
        x5_offset = const offset_of!(Thread, x5),
        x6_offset = const offset_of!(Thread, x6),
        x7_offset = const offset_of!(Thread, x7),
        x8_offset = const offset_of!(Thread, x8),
        x9_offset = const offset_of!(Thread, x9),
        x10_offset = const offset_of!(Thread, x10),
        x11_offset = const offset_of!(Thread, x11),
        x12_offset = const offset_of!(Thread, x12),
        x13_offset = const offset_of!(Thread, x13),
        x14_offset = const offset_of!(Thread, x14),
        x15_offset = const offset_of!(Thread, x15),
        x16_offset = const offset_of!(Thread, x16),
        x17_offset = const offset_of!(Thread, x17),
        x18_offset = const offset_of!(Thread, x18),
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
        x30_offset = const offset_of!(Thread, x30),
        elr_offset = const offset_of!(Thread, elr),
        handle_interrupt = sym handle_interrupt,
    );
}

// #[unsafe(naked)]
#[unsafe(no_mangle)]
pub extern "C" fn unreachable_handler() -> ! {
    unreachable!("unexpected exception");
}
