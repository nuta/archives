use crate::machine::build_bootinfo;
use arch_common::bootinfo::BootInfo;
use etc::addr_of_fn;

use crate::{
    asm,
    registers::{
        medeleg::Medeleg,
        mepc::Mepc,
        mstatus::{Mpp, Mstatus},
        pmpaddr0::PmpAddr0,
        pmpcfg0::PmpCfg0,
        satp::Satp,
        stvec::Stvec,
        PmpAddressMode,
    },
};

extern "C" {
    // Defined in trap.S.
    fn riscv64_trap_entry();
}

extern "Rust" {
    // The entrypoint defined in kernel crate.
    fn kernel_main(bootinfo: &BootInfo);
}

/// Called from the machine-specific boot code. The hart is still in M-mode.
pub unsafe fn arch_boot() -> ! {
    // Deliver all exceptions to S-Mode.
    Medeleg::from_raw(0xffff).write();

    // Register the trap handler.
    Stvec::new(addr_of_fn!(riscv64_trap_entry)).write();

    // Disable paging in S-mode for now.
    Satp::from_raw(0).write();

    // Allow S-Mode to access the whole physical memory. 0x3f_ffff_ffff_ffff is
    // the maximum value as of today (upper bits are reserved).
    PmpCfg0::new()
        .set_readable(true)
        .set_writable(true)
        .set_executable(true)
        .set_address_mode(PmpAddressMode::TopOfRange)
        .write();
    PmpAddr0::from_raw(0x3f_ffff_ffff_ffff).write();

    // Switch from the M-mode into the S-mode. The execution continues from
    // supervisor_mode_entry().
    Mstatus::read().set_mpp(Mpp::Supervisor).write();
    Mepc::from_raw(supervisor_mode_entry as *const () as usize).write();
    asm::mret();

    unreachable!();
}

/// Called from boot(). The hart is now in S-mode.
unsafe fn supervisor_mode_entry() {
    let bootinfo = build_bootinfo();
    kernel_main(&bootinfo);
}
