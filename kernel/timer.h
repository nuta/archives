#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include "event.h"
#include "channel.h"

struct timer {
    struct timer *next;
    struct event *event;
    size_t current;
    size_t reset;  // 0 on oneshot timer
};

void add_oneshot_timer(struct channel *ch, size_t ms, uintmax_t arg);
void add_interval_timer(struct channel *ch, size_t interval, uintmax_t arg);

void advance_clock(size_t ms);
size_t get_next_timeout(void);

#endif
