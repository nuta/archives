#include <resea.h>
#include <arch.h>
#include <kernel/timer.h>
#include <kernel/thread.h>
#include "hypercall_table.h"


unsigned long prev_millis = 0;

void arch_halt_until(size_t ms) {
    size_t from_prev_wait = hypercall_table->millis() - prev_millis;

    if (prev_millis == 0) {
        hypercall_table->delay(ms);
        advance_clock(ms);
    } else {
        if (from_prev_wait < ms) {
            hypercall_table->delay(ms - from_prev_wait);
            advance_clock(ms);
        } else {
            advance_clock(from_prev_wait);
        }
    }

    prev_millis = hypercall_table->millis();
    // return to yield()
}
