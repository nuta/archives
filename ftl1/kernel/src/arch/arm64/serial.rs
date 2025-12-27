//! # PL011 UART driver
//!
//! Spec: <https://developer.arm.com/documentation/ddi0183/g/programmers-model/summary-of-registers>

use core::ptr;

use super::paddr2vaddr;
use crate::address::PAddr;

const UART0_BASE: PAddr = PAddr::new(0x0900_0000);

#[derive(Clone, Copy)]
struct Reg(usize);

impl Reg {
    fn write(&self, value: u32) {
        unsafe {
            let base = paddr2vaddr(UART0_BASE).as_usize();
            ptr::write_volatile((base + self.0) as *mut u32, value);
        }
    }
}

/// Control Register.
const UARTCR: Reg = Reg(0x030);
/// Line Control Register.
const UARTLCR_H: Reg = Reg(0x02c);
/// Data Register.
const UARTDR: Reg = Reg(0x000);

pub fn init() {
    // Disable UART.
    UARTCR.write(0);
    // 8-bit word length, FIFO enabled.
    UARTLCR_H.write(0x70);
    // Enable UART.
    UARTCR.write(0x301);
}

pub fn console_write(bytes: &[u8]) {
    for &byte in bytes {
        UARTDR.write(byte as u32);
    }
}
