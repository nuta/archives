use ftl_types::device::IrqDesc;
use ftl_types::error::ErrorCode;

use crate::address::VAddr;
use crate::arch::get_cpuvar;
use crate::cpuvar::CpuId;

const IRQ_MAX: usize = 1024;

pub struct Reg(usize);

// Interrupt Source Priority
// https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc#3-interrupt-priorities
fn priority_reg_offset(irq: usize) -> Reg {
    Reg(4 * irq)
}

// Interrupt Enable Bits
// https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc#5-interrupt-enables
fn enable_reg_offset(irq: usize) -> Reg {
    Reg(0x2080 + (irq / 32 * size_of::<u32>()))
}

/// Interrupt Claim Register
/// https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc#7-interrupt-claim-process
fn claim_reg_offset(hart: CpuId) -> Reg {
    Reg(0x201004 + 0x2000 * hart.as_usize())
}

// Priority Threshold
// https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc#6-priority-thresholds
fn threshold_reg_offset(hart: CpuId) -> Reg {
    Reg(0x201000 + 0x2000 * hart.as_usize())
}

pub static INTERRUPT_CONTROLLER: Plic = Plic::new();

pub struct Plic {
    base: VAddr,
}

impl Plic {
    pub const fn new() -> Self {
        let base = VAddr::new(0xffff_ff80_0000_0000 + 0x0c00_0000); // FIXME:
        Self { base }
    }

    pub fn acquire(&self, irq: usize) -> Result<(), ErrorCode> {
        if irq >= IRQ_MAX {
            return Err(ErrorCode::InvalidArgument);
        }

        self.write_reg(priority_reg_offset(irq), 1);

        let mut value = self.read_reg(enable_reg_offset(irq));
        value |= 1 << ((irq as u32) % 32);
        self.write_reg(enable_reg_offset(irq), value);

        Ok(())
    }

    pub fn release(&self, _irq: usize) {
        debug_warn!("Plic::release: not implemented");
    }

    pub fn acknowledge(&self, irq: usize) {
        self.write_reg(claim_reg_offset(get_cpuvar().id), irq as u32);
    }

    pub fn get_pending_irq(&self) -> Option<usize> {
        let irq = self.read_reg(claim_reg_offset(get_cpuvar().id));
        Some(irq as usize)
    }

    pub(super) fn init_per_cpu(&self, cpu_id: CpuId) {
        // Enable all interrupts by setting the threshold to 0.
        self.write_reg(threshold_reg_offset(cpu_id), 0)
    }

    fn read_reg(&self, reg: Reg) -> u32 {
        unsafe {
            let ptr = self.base.add(reg.0).as_mut_ptr();
            core::ptr::read_volatile(ptr)
        }
    }

    fn write_reg(&self, reg: Reg, value: u32) {
        unsafe {
            let ptr = self.base.add(reg.0).as_mut_ptr();
            core::ptr::write_volatile(ptr, value);
        }
    }
}

pub fn irq_desc_from_devtree(node: &ftl_dtb_parser::Node<'_>) -> IrqDesc {
    let prop = node
        .props()
        .find(|prop| prop.name() == b"interrupts")
        .expect("interrupts property not found");
    let cells = prop.as_u32_array().expect("failed to parse interrupts");

    assert_eq!(cells.len(), 1);
    let irq_number: u32 = cells[0].into();
    IrqDesc::new(irq_number as usize)
}
