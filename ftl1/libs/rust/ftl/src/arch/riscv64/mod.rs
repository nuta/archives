use core::arch::asm;

use ftl_types::environ::VSyscallPage;

pub fn set_vsyscall_page(vsyscall: *const VSyscallPage) {
    unsafe {
        asm!("mv tp, {}", in(reg) vsyscall);
    }
}

pub fn get_vsyscall_page() -> &'static VSyscallPage {
    let tp: u64;
    unsafe {
        asm!("mv {}, tp", out(reg) tp);
        &*(tp as *const VSyscallPage)
    }
}
