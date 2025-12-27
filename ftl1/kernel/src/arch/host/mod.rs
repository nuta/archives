#![allow(static_mut_refs)] // Allow mutable static references for host testing

use core::ops::Range;

use ftl_types::device::IrqDesc;
use ftl_types::error::ErrorCode;
use ftl_types::timer::Instant;

use crate::address::PAddr;
use crate::address::UAddr;
use crate::address::VAddr;
use crate::cpuvar::CpuId;

pub const MIN_PAGE_SIZE: usize = 0x1000;
pub const UADDR_RANGE: Range<UAddr> = UAddr::new(0x10000)..UAddr::new(0x7fff_0000_0000);

pub use ftl_types::arch::PageAttrs;

pub fn paddr2vaddr(_paddr: PAddr) -> VAddr {
    todo!()
}

pub fn vaddr2paddr(_vaddr: VAddr) -> PAddr {
    todo!()
}

pub unsafe extern "C" fn kernelcall_handler() -> ! {
    todo!()
}

pub fn console_write(_bytes: &[u8]) {
    todo!()
}

pub fn halt() -> ! {
    todo!()
}

pub fn idle() -> ! {
    todo!()
}

pub fn read_monotonic_time() -> Instant {
    todo!()
}

pub fn set_timer(deadline: Instant) {
    todo!()
}

pub fn switch_into_thread(_thread: *mut Thread) -> ! {
    todo!()
}

pub fn init() {
    todo!()
}

pub fn get_cpuvar() -> &'static crate::cpuvar::CpuVar {
    todo!()
}

pub fn init_cpuvar(_init: crate::cpuvar::CpuVar) {
    todo!()
}

pub struct CpuVar {}

impl CpuVar {
    pub fn new() -> Self {
        todo!()
    }
}

pub struct Thread {}

impl Thread {
    pub fn new_idle() -> Self {
        todo!()
    }

    pub fn new(_entry: usize, _arg: usize, _sp: usize) -> Self {
        todo!()
    }

    pub fn set_syscall_result(&mut self, _retval: Result<usize, ErrorCode>) {
        todo!()
    }
}

pub struct VmSpace {}

impl VmSpace {
    pub fn new() -> Result<Self, ErrorCode> {
        Ok(Self {})
    }

    pub fn map(&self, _vaddr: VAddr, _paddr: PAddr, _attrs: PageAttrs) -> Result<(), ErrorCode> {
        Ok(())
    }

    pub fn switch(&self) {}
}

pub static INTERRUPT_CONTROLLER: InterruptController = InterruptController::new();

pub struct InterruptController {}

impl InterruptController {
    pub const fn new() -> Self {
        Self {}
    }

    pub fn init_per_cpu(&self, _cpu_id: CpuId) {
        todo!()
    }

    pub fn acquire(&self, _irq: usize) -> Result<(), ErrorCode> {
        todo!()
    }

    pub fn release(&self, _irq: usize) {
        todo!()
    }

    pub fn acknowledge(&self, _irq: usize) {
        todo!()
    }

    pub fn get_pending_irq(&self) -> Option<usize> {
        todo!()
    }
}

pub fn irq_desc_from_devtree(_devtree: &ftl_dtb_parser::Node<'_>) -> IrqDesc {
    todo!()
}
