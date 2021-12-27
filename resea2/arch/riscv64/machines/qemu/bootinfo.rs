use arch_common::{
    bootinfo::{BootInfo, FreeRamArea},
    paddr::PAddr,
    vaddr::VAddr,
};
use arrayvec::ArrayVec;
use etc::addr_of_var;

extern "C" {
    // A liker-generated symbol which points to the end of the kernel memory
    // space. Defined in kernel.ld.
    static __kernel_end: u8;
}

pub fn build_bootinfo() -> BootInfo {
    let free_ram_start = unsafe { addr_of_var!(__kernel_end) };

    // Assuming QEMU provides at least 128MiB.
    // FIXME: Get the end of RAM from Device Tree.
    let free_ram_end = 0x8000_0000 + 128 * 1024 * 1024;

    let mut free_ram_areas = ArrayVec::new();
    free_ram_areas.push(FreeRamArea {
        // SAFETY: Assuming the physical memory addresses are straight-mapped
        //         into the kernel's page table and all pages are mapped.
        vaddr: unsafe { VAddr::new(free_ram_start) },
        paddr: PAddr::new(free_ram_start),
        len: free_ram_end - free_ram_start,
    });
    BootInfo { free_ram_areas }
}
