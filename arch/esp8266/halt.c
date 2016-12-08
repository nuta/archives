#include <resea.h>
#include <arch.h>
#include <kernel/timer.h>
#include <kernel/thread.h>
#include "finfo.h"


unsigned long prev_millis = 0;

void arch_halt_until(size_t ms) {
    size_t from_prev_wait = finfo->millis() - prev_millis;

    prev_millis = finfo->millis();

    if (prev_millis == 0) {
        finfo->delay(ms);
        advance_clock(ms);
    } else {
        if (from_prev_wait < ms) {
            finfo->delay(ms - from_prev_wait);
            advance_clock(ms);
        } else {
            advance_clock(from_prev_wait);
        }
    }

    // return to yield()
}
