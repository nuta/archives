#include <arch.h>
#include <x64/serial.h>
#include <x64/x64.h>

void x64_serial_putchar(char ch) {
    // Wait until the serial port gets ready.
    while ((asm_in8(IOPORT_SERIAL + LSR) & TX_READY) == 0) {}
    asm_out8(IOPORT_SERIAL, ch);
}

int arch_debugger_readchar(void) {
    if ((asm_in8(IOPORT_SERIAL + LSR) & 1) == 0) {
        return -1;
    }

    return asm_in8(IOPORT_SERIAL + RBR);
}

void x64_serial_early_init(void) {
    int baud = 9600;
    int divisor = 115200 / baud;
    asm_out8(IOPORT_SERIAL + IER, 0x00); // Disable interrupts.
    asm_out8(IOPORT_SERIAL + DLL, divisor & 0xff);
    asm_out8(IOPORT_SERIAL + DLH, (divisor >> 8) & 0xff);
    asm_out8(IOPORT_SERIAL + LCR, 0x03); // 8n1.
    asm_out8(IOPORT_SERIAL + FCR, 0x00); // No FIFO.
}

void x64_serial_init(void) {
    asm_out8(IOPORT_SERIAL + IER, 0x01); // Enable interrupts.
    enable_irq(SERIAL_IRQ);
}
