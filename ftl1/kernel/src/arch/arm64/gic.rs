//! Generic Interrupt Controller (GIC) version 2 support.
use core::mem::size_of;

use ftl_types::device::IrqDesc;
use ftl_types::error::ErrorCode;

use super::paddr2vaddr;
use crate::address::PAddr;
use crate::cpuvar::CpuId;

const IRQ_MAX: usize = 1024;

const GICD_CTLR: usize = 0x000;
const GICD_ISENABLER: usize = 0x100;
const GICD_IPRIORITYR: usize = 0x400;
const GICD_ITARGETSR: usize = 0x800;

const GICC_CTLR: usize = 0x000;
const GICC_PMR: usize = 0x004;
const GICC_IAR: usize = 0x00c;
const GICC_EOIR: usize = 0x010;

#[derive(Clone, Copy)]
pub struct Reg(usize);

pub static INTERRUPT_CONTROLLER: Gic = Gic::new();

pub struct Gic {
    gicd_base: PAddr,
    gicc_base: PAddr,
}

impl Gic {
    pub const fn new() -> Self {
        let gicd_base = PAddr::new(0x0800_0000);
        let gicc_base = PAddr::new(0x0801_0000);
        Self {
            gicd_base,
            gicc_base,
        }
    }

    pub fn init_per_cpu(&self, cpu_id: CpuId) {
        if cpu_id.as_usize() == 0 {
            self.write_gicd_reg(Reg(GICD_CTLR), 0);

            self.write_gicc_reg(Reg(GICC_PMR), 255);

            self.write_gicc_reg(Reg(GICC_CTLR), 1);
            self.write_gicd_reg(Reg(GICD_CTLR), 1);
        }
    }

    pub fn acquire(&self, irq: usize) -> Result<(), ErrorCode> {
        if irq >= IRQ_MAX {
            return Err(ErrorCode::InvalidArgument);
        }

        let irq_shift = (irq % 4) * 8;

        // Enable the interrupt.
        {
            let offset = irq / 32;
            let mut value = self.read_gicd_reg(Reg(GICD_ISENABLER + offset * size_of::<u32>()));
            value |= 1 << (irq % 32);
            self.write_gicd_reg(Reg(GICD_ISENABLER + offset * size_of::<u32>()), value);
        }

        // Set the priority of the interrupt to the highest.
        {
            let offset = irq / 4;
            let mut value = self.read_gicd_reg(Reg(GICD_IPRIORITYR + offset * size_of::<u32>()));
            value &= !(0xff << irq_shift);
            self.write_gicd_reg(Reg(GICD_IPRIORITYR + offset * size_of::<u32>()), value);
        }

        // Set the target processor to the first processor.
        // TODO: Multi-processor support.
        {
            let target = 0;
            let offset = irq / 4;
            let mut value = self.read_gicd_reg(Reg(GICD_ITARGETSR + offset * size_of::<u32>()));
            value &= !(0xff << irq_shift);
            value |= (1 << target) << irq_shift;
            self.write_gicd_reg(Reg(GICD_ITARGETSR + offset * size_of::<u32>()), value);
        }

        Ok(())
    }

    pub fn release(&self, _irq: usize) {
        debug_warn!("Gic::release: not implemented");
    }

    pub fn acknowledge(&self, irq: usize) {
        self.write_gicc_reg(Reg(GICC_EOIR), irq as u32);
    }

    pub fn get_pending_irq(&self) -> Option<usize> {
        let iar = self.read_gicc_reg(Reg(GICC_IAR));
        let irq = (iar & 0x3ff) as usize;
        if irq >= 1020 { None } else { Some(irq) }
    }

    fn gicd_reg(&self, reg: Reg) -> *mut u32 {
        let vaddr = paddr2vaddr(self.gicd_base.checked_add(reg.0).unwrap());
        vaddr.as_mut_ptr()
    }

    fn read_gicd_reg(&self, reg: Reg) -> u32 {
        unsafe { core::ptr::read_volatile(self.gicd_reg(reg)) }
    }

    fn write_gicd_reg(&self, reg: Reg, value: u32) {
        unsafe {
            core::ptr::write_volatile(self.gicd_reg(reg), value);
        }
    }

    fn gicc_reg(&self, reg: Reg) -> *mut u32 {
        let vaddr = paddr2vaddr(self.gicc_base.checked_add(reg.0).unwrap());
        vaddr.as_mut_ptr()
    }

    fn read_gicc_reg(&self, reg: Reg) -> u32 {
        unsafe { core::ptr::read_volatile(self.gicc_reg(reg)) }
    }

    fn write_gicc_reg(&self, reg: Reg, value: u32) {
        unsafe {
            core::ptr::write_volatile(self.gicc_reg(reg), value);
        }
    }
}

pub fn irq_desc_from_devtree(node: &ftl_dtb_parser::Node<'_>) -> IrqDesc {
    let prop = node
        .props()
        .find(|prop| prop.name() == b"interrupts")
        .expect("interrupts property not found");
    let cells = prop.as_u32_array().expect("failed to parse interrupts");

    assert_eq!(cells.len(), 3);
    let irq_type: u32 = cells[0].into();
    let irq_number: u32 = cells[1].into();
    let irq = match irq_type {
        0 => irq_number + 32, // SPI
        1 => irq_number + 16, // PPI
        _ => unreachable!("unexpected irq type: {}", irq_type),
    };

    IrqDesc::new(irq as usize)
}
