#include "types.h"
#include "thread.h"
#include "printk.h"

void tick_timer(void) {
    static int tick = 0;

    tick++;
    if (tick > THREAD_SWITCH_INTERVAL) {
        tick = 0;
        thread_switch();
    }
}
