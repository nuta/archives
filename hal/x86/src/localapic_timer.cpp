#include "x86.h"
#include <resea.h>
#include <hal.h>
#include "x86.h"


void x86_init_localapic_timer(void) {
    uint8_t vector;

    x86_set_int_desc((struct int_desc *) &CPUVAR->idt[32], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) x86_asm_int_handler32);

    vector = x86_alloc_intr_vector();
    x86_ioapic_enable_irq(vector, 0);

    write_localapic(LOCALAPIC_REG_TIMER_INITCNT, 10000);
    write_localapic(LOCALAPIC_REG_LVT_TIMER, vector | 0x20000);
    write_localapic(LOCALAPIC_REG_TIMER_DIV, 0x03);

    x86_timer_vector = vector;
}

