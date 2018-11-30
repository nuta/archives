#include <kernel/printk.h>
#include "apic.h"
#include "ioapic.h"
#include "handler.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "asm.h"
#include "msr.h"

void x64_ack_interrupt(void) {
    x64_write_apic(APIC_REG_EOI, 0);
}


#define PIT_HZ 1193182
#define PIT_CH2 0x42
#define PIT_CMD 0x43
#define KBC_PORT_B 0x61
#define KBC_B_OUT2_STATUS 0x20
#define APIC_TIMER_DIV 0x03

static void calibrate_apic_timer(void) {
    // FIXME: This method works well on QEMU though on the real machine
    // this doesn't. For example on my machine `counts_per_sec' is 7x greater
    // than the desired value.

    // initialize PIT
    int freq = 1000; /* That is, every 1ms. */
    u16_t divider = PIT_HZ / freq;
    asm_outb(KBC_PORT_B, asm_inb(KBC_PORT_B) & 0xfc /* Disable ch #2 and speaker output */);
    asm_outb(PIT_CMD, 0xb2 /* Select ch #2 */);
    asm_outb(PIT_CH2, divider & 0xff);
    asm_outb(PIT_CH2, (divider >> 8) & 0xff);
    asm_outb(KBC_PORT_B, asm_inb(KBC_PORT_B) | 1 /* Enable ch #2 */);

    // Reset the counter in APIC timer.
    x64_write_apic(APIC_REG_TIMER_INITCNT, 0xffffffff);

    // Wait for the PIT.
    while ((asm_inb(KBC_PORT_B) & KBC_B_OUT2_STATUS) == 0);

    u32_t counts_per_sec = ((0xffffffff - x64_read_apic(APIC_REG_TIMER_CURRENT)) * freq) << APIC_TIMER_DIV;
    x64_write_apic(APIC_REG_TIMER_INITCNT, counts_per_sec / TICK_HZ);
    DEBUG("calibrated the APIC timer using PIT: %d counts/msec", counts_per_sec / 1000);
}


void x64_init_apic_timer(void) {
    x64_ioapic_enable_irq(APIC_TIMER_VECTOR, 0);
    x64_write_apic(APIC_REG_TIMER_INITCNT, 0xffffffff);
    x64_write_apic(APIC_REG_LVT_TIMER, APIC_TIMER_VECTOR | 0x20000);
    x64_write_apic(APIC_REG_TIMER_DIV, APIC_TIMER_DIV);
    calibrate_apic_timer();
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
