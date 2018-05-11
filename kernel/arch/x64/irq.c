#include <kernel/types.h>
#include <kernel/thread.h>
#include <kernel/printk.h>
#include "apic.h"
#include "irq.h"


void x64_handle_unkown_irq(void) {
    x64_ack_interrupt();
    INFO("Unknown Interrupt");
}


void x64_handle_irq(UNUSED u8_t vector) {
    static int tick = 0;

    x64_ack_interrupt();

    if (tick++ > THREAD_SWITCH_INTERVAL) {
        tick = 0;
        thread_switch();
    }
}
