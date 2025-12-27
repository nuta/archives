use core::arch::asm;
use core::ptr;

use crate::arch::arm64::boot::KERNEL_STACK_SIZE;

const CPUVAR_MAGIC: u64 = 0xc12f_c12f_c12f_c12f;

pub struct CpuVar {
    magic: u64,
}

impl CpuVar {
    pub fn new() -> Self {
        Self {
            magic: CPUVAR_MAGIC,
        }
    }
}

const SP_BOTTOM_MASK: u64 = !(KERNEL_STACK_SIZE as u64 - 1);

fn get_cpuvar_ptr() -> *mut crate::cpuvar::CpuVar {
    let sp: u64;
    unsafe {
        asm!("mov {}, sp", out(reg) sp);
    }

    let stack_bottom = sp & SP_BOTTOM_MASK;
    stack_bottom as *mut crate::cpuvar::CpuVar
}

pub fn get_cpuvar() -> &'static crate::cpuvar::CpuVar {
    let cpuvar = get_cpuvar_ptr();
    debug_assert_eq!(unsafe { (*cpuvar).arch.magic }, CPUVAR_MAGIC);
    unsafe { &*cpuvar }
}

pub fn init_cpuvar(value: crate::cpuvar::CpuVar) {
    unsafe {
        ptr::write(get_cpuvar_ptr(), value);
    }
}
