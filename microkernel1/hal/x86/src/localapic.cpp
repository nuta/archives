#include "x86.h"
#include <hal.h>


static uint32_t read_localapic (paddr_t addr) {

    return *((volatile uint32_t *) PHY(addr));
}


static void write_localapic (paddr_t addr, uint32_t data) {

    *((volatile uint32_t *) PHY(addr)) = data;
    return;
}


void x86_ack_interrupt (void) {

    write_localapic(LOCALAPIC_REG_EOI, 0);
    return;
}


cpuid_t hal_get_cpuid (void) {

    return (read_localapic(LOCALAPIC_REG_ID) >> 24);
}

void x86_init_localapic (void) {

    // Local APIC hardware intialization
    x86_asm_wrmsr(MSR_IA32_APIC_BASE,
                  (x86_asm_rdmsr(MSR_IA32_APIC_BASE) & 0xfffff100) | 0x0800);

    // Local APIC software initialization
    write_localapic(LOCALAPIC_REG_SPURIOUS_INT, 1 << 8);

    // task priority
    write_localapic(LOCALAPIC_REG_TPR, 0);

    // logical destination register
    write_localapic(LOCALAPIC_REG_LOGICAL_DEST, 0x01000000);

    // destination format register
    write_localapic(LOCALAPIC_REG_DEST_FORMAT, 0xffffffff);

    // timer interrupt
    write_localapic(LOCALAPIC_REG_LVT_TIMER, 1 << 16 /* masked */);

    // errror interrupt
    write_localapic(LOCALAPIC_REG_LVT_ERROR, 1 << 16 /* masked */);
}


void x86_init_localapic_timer(void) {
    uint8_t vector;

    vector = x86_alloc_intr_vector();

    x86_set_int_desc((struct int_desc *) &CPUVAR->idt[vector], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) x86_asm_int_handler32);

    x86_ioapic_enable_irq(vector, 0);

    write_localapic(LOCALAPIC_REG_TIMER_INITCNT, 10000);
    write_localapic(LOCALAPIC_REG_LVT_TIMER, vector | 0x20000);
    write_localapic(LOCALAPIC_REG_TIMER_DIV, 0x03);

    x86_timer_vector = vector;
}
