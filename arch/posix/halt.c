#include <arch.h>
#include <kernel/timer.h>


void arch_halt_until(size_t ms) {

    usleep(ms * 1000);
    advance_clock(ms);
}
