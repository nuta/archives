#include "kmalloc.h"
#include "channel.h"
#include "event.h"


static struct channel *listeners = NULL;

void listen_event(struct channel *ch, msgid_t type, uintmax_t arg) {

    struct event *new_e = kmalloc(sizeof(*new_e), KMALLOC_NORMAL);
    new_e->next  = NULL;
    new_e->type  = type;
    new_e->flags = 0;
    new_e->arg   = arg;

    ch->flags |= CHANNEL_EVENT;

    mutex_lock(&ch->receiver_lock);

    struct event **e = &ch->events;
    while (*e)
        e = &((*e)->next);
    *e = new_e;

    struct channel **c = &listeners;
    while (*c)
        c = &((*c)->next);
    *c = ch;

    mutex_unlock(&ch->receiver_lock);
}


void fire_event(msgid_t type) {

    struct channel *ch = listeners;
    while (ch) {
        struct event *e = ch->events;

        while (e) {
            if (e->type == type) {
                e->flags |= EVENT_FIRED;
                DEBUG("fired the event 0x%x", type);
                return;
            }

            e = e->next;
        }

        ch = ch->next;
    }

    DEBUG("no event listener of the event 0x%x", type);
}
