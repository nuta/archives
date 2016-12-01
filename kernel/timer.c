#include <resea/timer.h>
#include "event.h"
#include "kmalloc.h"
#include "list.h"
#include "timer.h"


static size_t next_timeout = SIZE_MAX;
static struct timer *timers = NULL;
static mutex_t timers_lock = MUTEX_INITIALIZER;


static void add_timer(struct channel *ch, size_t current, size_t reset,
                      uintmax_t arg) {

    listen_event(ch, TIMER_TIMEOUT, arg);

    struct timer *timer = kmalloc(sizeof(*timer), KMALLOC_NORMAL);
    timer->ch      = ch;
    timer->current = current;
    timer->reset   = reset;

    mutex_lock(&timers_lock);
    insert_into_list((struct list **) &timers, timer);
    mutex_unlock(&timers_lock);
}


void add_interval_timer(struct channel *ch, size_t interval, uintmax_t arg) {

    add_timer(ch, interval, interval, arg);
}


void add_oneshot_timer(struct channel *ch, size_t ms, uintmax_t arg) {

    add_timer(ch, ms, 0, arg);
}


void advance_clock(size_t ms) {

    size_t next = (timers)? timers->current : 1000;

    for (struct timer *t = timers; t; t = t->next) {

        if (t->current < ms) {
            fire_event_to(t->ch, TIMER_TIMEOUT);

            if (t->reset)
                remove_from_list((struct list **) &timers, t);

            t->current = t->reset;
        } else {
            t->current -= ms;
            if (t->current < next)
                next = t->current;
        }
    }

    next_timeout = next;
}


size_t get_next_timeout(void) {

    return (next_timeout == SIZE_MAX) ? 0 : next_timeout;
}
