use crate::address::VAddr;
use crate::cpuvar;
use crate::device_tree;
use crate::memory;
use crate::startup;
use crate::thread;
use crate::vmspace;

pub struct BootInfo {
    pub cpu_id: cpuvar::CpuId,
    pub dtb: VAddr,
}

pub fn boot(bootinfo: BootInfo) -> ! {
    info!("Booting FTL...");

    memory::init();
    let dtb = device_tree::init(bootinfo.dtb);
    vmspace::init();
    cpuvar::init(bootinfo.cpu_id);
    startup::init(dtb);

    // Switch to the first thread.
    thread::return_to_user();
}
