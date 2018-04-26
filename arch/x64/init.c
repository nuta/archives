#include <resea/types.h>

static inline void asm_outb(uint16_t port, uint8_t value) {
    INLINE_ASM("outb %0, %1" :: "a"(value), "Nd"(port));
}

#define IOPORT_SERIAL 0x3f8
static void serial_init(void) {
    asm_outb(IOPORT_SERIAL + 1, 0x00);
    asm_outb(IOPORT_SERIAL + 3, 0x80);
    asm_outb(IOPORT_SERIAL + 0, 0x03);
    asm_outb(IOPORT_SERIAL + 1, 0x00);
    asm_outb(IOPORT_SERIAL + 3, 0x03);
    asm_outb(IOPORT_SERIAL + 2, 0xC7);
    asm_outb(IOPORT_SERIAL + 4, 0x0B);
}

void arch_putchar(char ch) {
    asm_outb(IOPORT_SERIAL, ch);
}


extern uint8_t __bss_start;
extern uint8_t __bss_end;

void x64_init(void) {
    /* Clear .bss section. */
    for (uint8_t *p = &__bss_start; p < &__bss_end; p++) {
        *p = 0;
    }

    serial_init();
    kernel_init();
}
