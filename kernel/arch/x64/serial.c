#include "asm.h"

#define IOPORT_SERIAL 0x3f8
#define IER 1
#define DLL 0
#define DLH 1
#define LCR 3
#define FCR 2
#define LSR 5
#define TX_READY 0x20

void x64_init_serial(void) {
    int baud = 9600;
    int divisor = 115200 / baud;

    asm_outb(IOPORT_SERIAL + IER, 0x00); // Disable interrupts

    asm_outb(IOPORT_SERIAL + DLL, divisor & 0xff);
    asm_outb(IOPORT_SERIAL + DLH, (divisor >> 8) & 0xff);

    asm_outb(IOPORT_SERIAL + LCR, 0x03); // 8n1
    asm_outb(IOPORT_SERIAL + FCR, 0x00); // No FIFO
}


void arch_putchar(char ch) {
    while ((asm_inb(IOPORT_SERIAL + LSR) & TX_READY) == 0);

    asm_outb(IOPORT_SERIAL, ch);
}
