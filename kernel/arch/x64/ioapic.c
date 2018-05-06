#include "asm.h"
#include "cpu.h"
#include "gdt.h"
#include "idt.h"
#include "handler.h"
#include "ioapic.h"

static paddr_t mmio;

static u32_t ioapic_read(u8_t reg) {

    *((u32_t *) from_paddr(mmio)) = reg;
    return *((u32_t *) from_paddr(mmio + 0x10));
}


static void ioapic_write(u8_t reg, u32_t data) {

    *((u32_t *) from_paddr(mmio)) = reg;
    *((u32_t *) from_paddr(mmio + 0x10)) = data;
}


void x64_ioapic_enable_irq(u8_t vector, u8_t irq) {

    ioapic_write(IOAPIC_REG_NTH_IOREDTBL_HIGH(irq), 0);
    ioapic_write(IOAPIC_REG_NTH_IOREDTBL_LOW(irq),  vector);
}


void x64_init_ioapic(paddr_t mmio_addr) {
    int max;
    mmio = mmio_addr;

    // symmetric I/O mode
    asm_outb(0x22, 0x70);
    asm_outb(0x23, 0x01);

    // get the maxinum number of entries in IOREDTBL
    max = (ioapic_read(IOAPIC_REG_IOAPICVER) >> 16) + 1;

    // disable all hardware interrupts
    for (int i=0; i < max; i++) {
        ioapic_write(IOAPIC_REG_NTH_IOREDTBL_HIGH(i), 0);
        ioapic_write(IOAPIC_REG_NTH_IOREDTBL_LOW(i),  1 << 16 /* masked */);
    }
}
