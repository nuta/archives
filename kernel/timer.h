#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

struct timer {
    struct timer *next;
    struct channel *ch;
    size_t current;
    size_t reset;  // 0 on oneshot timer
};

void add_timer(struct channel *ch, size_t ms);
void advance_clock(size_t ms);

#endif
