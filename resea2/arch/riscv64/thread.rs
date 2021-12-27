#[cfg(target_arch = "riscv64")]
mod assembly_files {
    global_asm!(include_str!("switch.S"));
}

use core::cell::UnsafeCell;

use arch_common::vaddr::VAddr;

#[must_use]
unsafe fn push_stack(mut sp: *mut u64, value: u64) -> *mut u64 {
    sp = sp.sub(1);
    sp.write(value);
    sp
}

pub struct ArchThread {
    sp: UnsafeCell<u64>,
}

unsafe impl Sync for ArchThread {}

impl ArchThread {
    pub fn new_kernel_thread(entry: VAddr, stack_top: VAddr) -> ArchThread {
        let sp = unsafe {
            let mut sp: *mut u64 = stack_top.as_mut_ptr();
            sp = sp.add(8);
            sp = push_stack(sp, entry.as_usize() as u64);
            sp as u64
        };

        ArchThread {
            sp: UnsafeCell::new(sp),
        }
    }

    pub fn new_idle_thread() -> ArchThread {
        ArchThread {
            sp: UnsafeCell::new(0),
        }
    }
}

extern "C" {
    pub fn riscv64_switch_thread(prev_sp: *mut u64, next_sp: *mut u64);
}

pub fn switch_thread(prev: &ArchThread, next: &ArchThread) {
    unsafe {
        riscv64_switch_thread(prev.sp.get(), next.sp.get());
    }
}
