#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/printk.h>
#include "apic.h"
#include "ioapic.h"
#include "irq.h"


void x64_handle_unkown_irq(void) {
    x64_ack_interrupt();
    INFO("Unknown Interrupt");
}


void x64_handle_irq(UNUSED u8_t vector) {
    x64_ack_interrupt();

    if (vector == APIC_TIMER_VECTOR) {
        tick_timer();
    } else {
        handle_irq(vector - VECTOR_IRQ_BASE);
    }
}


void arch_accept_irq(int irq) {
    if (irq > 32) {
        WARN("%s: too large IRQ vector (%d)", __func__, irq);
        return;
    }

    x64_ioapic_enable_irq(VECTOR_IRQ_BASE + irq, irq);
}
