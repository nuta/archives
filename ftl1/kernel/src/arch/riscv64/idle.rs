use core::arch::asm;

use crate::arch::riscv64::trap::handle_trap;
use crate::arch::riscv64::trap::trap_handler;

pub fn halt() -> ! {
    loop {
        unsafe {
            asm!("wfi");
        }
    }
}

/// The entry point of interrupts or exceptions.
#[rustc_align(4)]
unsafe extern "C" fn idle_entry() -> ! {
    // Restore the default handler. We don't need to save any
    // registers - the idle context will be entered by calling
    // arch::idle() directly.
    warn!("idle_entry");
    unsafe {
        asm!("csrw stvec, {}", in(reg) trap_handler as *const () as usize);
    }

    handle_trap();
}

pub fn idle() -> ! {
    trace!("entering idle");
    unsafe {
        // Switch the trap entry point because the default one will
        // overwrites the user registers with this idle context.
        asm!("csrw stvec, {}", in(reg) idle_entry as *const () as usize);

        // Memory fence to ensure writes so far become visible to other cores,
        // before entering WFI.
        asm!("fence");

        // Enable interrupts.
        asm!("csrsi sstatus, 1 << 1");
    }

    // Wait for something to happen...
    loop {
        unsafe {
            asm!("wfi");
        }
    }
}
