#include "_x86.h"
#include <hal.h>


// TODO: support multiple I/O APIC
static paddr_t mmio;

static uint32_t ioapic_read (uint8_t reg) {

    *((uint32_t *) PHY(mmio)) = reg;
    return *((uint32_t *) PHY(mmio + 0x10));
}


static void ioapic_write (uint8_t reg, uint32_t data) {

    *((uint32_t *) PHY(mmio))        = reg;
    *((uint32_t *) PHY(mmio + 0x10)) = data;
}


void x86_ioapic_enable_irq (uint8_t vector, uint8_t irq) {
    struct int_desc *idt;
    paddr_t handler;

    idt = (struct int_desc *) CPUVAR->idt;

    // compute the address of the interrupt handler
    handler  = (paddr_t) x86_asm_int_handler32 +
               (((paddr_t) x86_asm_int_handler34 -
                 (paddr_t) x86_asm_int_handler33) * irq);

    x86_set_int_desc((struct int_desc *) &idt[vector], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, handler);

    ioapic_write(IOAPIC_REG_NTH_IOREDTBL_HIGH(irq), 0);
    ioapic_write(IOAPIC_REG_NTH_IOREDTBL_LOW(irq),  vector);
}


void x86_init_ioapic (paddr_t _mmio) {
    int max;

    mmio = _mmio;

    // symmetric I/O mode
    x86_asm_out8(0x22, 0x70);
    x86_asm_out8(0x23, 0x01);

    // get the maxinum number of entries in IOREDTBL
    max = (ioapic_read(IOAPIC_REG_IOAPICVER) >> 16) + 1;

    // disable all hardware interrupts
    for (int i=0; i < max; i++) {
        ioapic_write(IOAPIC_REG_NTH_IOREDTBL_HIGH(i), 0);
        ioapic_write(IOAPIC_REG_NTH_IOREDTBL_LOW(i),  1 << 16 /* masked */);
    }
}
