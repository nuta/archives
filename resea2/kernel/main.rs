#![cfg_attr(not(test), no_std)]
#![cfg_attr(not(test), no_main)]
#![feature(alloc_error_handler)]

extern crate alloc;

#[macro_use]
extern crate print;

mod memory;
mod panic;
mod thread;

mod arch {
    #[cfg(feature = "riscv64")]
    pub use riscv64_arch::*;
    #[cfg(feature = "hosting")]
    pub use skelton_arch::*;
}

use arch_common::bootinfo::BootInfo;

#[no_mangle]
pub fn kernel_main(bootinfo: &BootInfo) {
    println!("Hello World from RISC-V!");

    memory::init(bootinfo);
    thread::init();

    #[allow(clippy::empty_loop)]
    loop {}
}
