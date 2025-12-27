use core::arch::asm;

use ftl_types::environ::VSyscallPage;

pub fn set_vsyscall_page(vsyscall: *const VSyscallPage) {
    unsafe {
        asm!("msr tpidr_el0, {}", in(reg) vsyscall);
    }
}

pub fn get_vsyscall_page() -> &'static VSyscallPage {
    let tpidr: u64;
    unsafe {
        asm!("mrs {}, tpidr_el0", out(reg) tpidr);
        &*(tpidr as *const VSyscallPage)
    }
}
