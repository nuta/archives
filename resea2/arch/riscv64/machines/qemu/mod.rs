#[cfg(target_arch = "riscv64")]
mod assembly_files {
    global_asm!(include_str!("boot.S"));
}

mod boot;
mod bootinfo;
mod serial;

pub const PAGE_SIZE: usize = 4096;
pub use bootinfo::build_bootinfo;
