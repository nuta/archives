//! A device driver for 16550 UART serial port.
//!
//! Datasheet: http://byterunner.com/16550.html
use crate::mmio::{ReadOnlyReg, ReadableReg, WritableReg, WriteOnlyReg};

/// The MMIO base physical address of UART0.
const UART0_BASE: usize = 0x10000000;
/// Transmit Holding Register.
const THR: WriteOnlyReg<u8> = WriteOnlyReg::new(UART0_BASE);
/// Interrupt Enable Register.
const IER: WriteOnlyReg<u8> = WriteOnlyReg::new(UART0_BASE + 1);
/// FIFO Control Register.
const FCR: WriteOnlyReg<u8> = WriteOnlyReg::new(UART0_BASE + 2);
/// Line Control Register.
const LCR: WriteOnlyReg<u8> = WriteOnlyReg::new(UART0_BASE + 3);
/// Line Status Register.
const LSR: ReadOnlyReg<u8> = ReadOnlyReg::new(UART0_BASE + 5);
///  Divisor Latch (LSB) Register.
const DLL: WriteOnlyReg<u8> = WriteOnlyReg::new(UART0_BASE);
///  Divisor Latch (MSB) Register.
const DLM: WriteOnlyReg<u8> = WriteOnlyReg::new(UART0_BASE + 1);

/// Enable RX/TX FIFOs.
const FCR_FIFO_ENABLE: u8 = 1 << 0;
/// Clear the RX FIFO.
const FCR_CLEAR_RX: u8 = 1 << 1;
/// Clear the TX FIFO.
const FCR_CLEAR_TX: u8 = 1 << 2;
/// The line format: 8 data bits and 1 stop bit.
/// https://en.wikipedia.org/wiki/8-N-1
const LCR_FORMAT_8N1: u8 = 3;
/// Enter the baud rate setting mode.
const LCR_BAUD_LATCH: u8 = 1 << 7;
/// TX FIFO is not full if it's set.
const LSR_TX_NOT_FULL: u8 = 1 << 5;

/// The baud rate divisor.
const BAUD_RATE_DIVISOR: u16 = 3;

#[no_mangle]
pub extern "C" fn printchar(c: u8) {
    unsafe {
        // Wait until TX FIFO is full.
        while LSR.read() & LSR_TX_NOT_FULL == 0 {}

        // Transmit a character.
        THR.write(c);
    }
}

pub unsafe fn init() {
    // Disable interrupts before the initialization.
    IER.write(0);
    // Configure the baud rate.
    LCR.write(LCR_BAUD_LATCH);
    DLL.write((BAUD_RATE_DIVISOR & 0xff) as u8);
    DLM.write(((BAUD_RATE_DIVISOR >> 8) & 0xff) as u8);
    // Leave the baud rate setting mode and configure the format.
    LCR.write(LCR_FORMAT_8N1);
    // Reset and enable FIFOs.
    FCR.write(FCR_FIFO_ENABLE | FCR_CLEAR_RX | FCR_CLEAR_TX);
}
