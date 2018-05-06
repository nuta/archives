#include "apic.h"
#include "ioapic.h"
#include "cpu.h"
#include "handler.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "asm.h"
#include "msr.h"

void x64_ack_interrupt(void) {
    x64_write_apic(APIC_REG_EOI, 0);
}


void x64_init_apic_timer(void) {
    u8_t vector = 32;
    x64_ioapic_enable_irq(vector, 0);
    x64_write_apic(APIC_REG_TIMER_INITCNT, 10000);
    x64_write_apic(APIC_REG_LVT_TIMER, vector | 0x20000);
    x64_write_apic(APIC_REG_TIMER_DIV, 0x03);
}


void x64_init_apic(void) {

    // Local APIC hardware intialization
    asm_wrmsr(MSR_APIC_BASE, (asm_rdmsr(MSR_APIC_BASE) & 0xfffff100) | 0x0800);

    // Local APIC software initialization
    x64_write_apic(APIC_REG_SPURIOUS_INT, 1 << 8);

    // task priority
    x64_write_apic(APIC_REG_TPR, 0);

    // logical destination register
    x64_write_apic(APIC_REG_LOGICAL_DEST, 0x01000000);

    // destination format register
    x64_write_apic(APIC_REG_DEST_FORMAT, 0xffffffff);

    // timer interrupt
    x64_write_apic(APIC_REG_LVT_TIMER, 1 << 16 /* masked */);

    // errror interrupt
    x64_write_apic(APIC_REG_LVT_ERROR, 1 << 16 /* masked */);
}
