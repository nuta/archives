use core::arch::asm;

use crate::arch::arm64::exception::idle_vector_table;

pub fn idle() -> ! {
    let vbar_addr = &raw const idle_vector_table as usize;

    trace!("entering idle");
    unsafe {
        asm!("msr vbar_el1, {}", in(reg) vbar_addr);
        asm!("msr daifclr, #2");
    }

    loop {
        unsafe {
            asm!("wfi");
        }
    }
}

pub fn halt() -> ! {
    loop {
        unsafe {
            asm!("wfi");
        }
    }
}
