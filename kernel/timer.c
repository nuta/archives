#include "event.h"
#include "list.h"
#include "timer.h"


static struct timer *timers = NULL;
static mutex_t timer_lock = MUTEX_INITIALIZER;

void add_interval_timer(struct channel *ch, size_t interval_ms, uintmax_t arg) {

    listen_event(ch, TIMER_INTERVAL, arg);

    struct timer *timer = kmalloc(sizeof(*timer), KMALLOC_NORMAL);
    timer->ch = ch;
    timer->current = interval_ms;
    timer->reset   = interval_ms;

    mutex_lock(&ch->receiver_lock);
    insert_into_list(&timers, timer);
    mutex_unlock(&ch->receiver_lock);
}


void advance_clock(size_t ms) {

    for (struct timer *t = timers; t; t = t->next) {
        if (t->current < ms) {
            fire_event(TIMER_TIMEOUT, t->ch);

            if (t->reset)
                remove_from_list(&timers, t);

            t->current = t->reset;
        } else {
            t->current -= ms;
        }
    }
}
