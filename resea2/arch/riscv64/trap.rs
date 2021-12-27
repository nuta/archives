#[cfg(target_arch = "riscv64")]
mod assembly_files {
    global_asm!(include_str!("trap.S"));
}

#[no_mangle]
pub extern "C" fn riscv64_trap_handler() {
    panic!("Trapped!");
}
