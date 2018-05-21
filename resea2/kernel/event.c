#include "kmalloc.h"
#include "thread.h"
#include "channel.h"
#include "event.h"
#include "list.h"


static struct listener *listeners = NULL;

struct event *listen_event(struct channel *ch, msgid_t type, uintmax_t arg) {

    if (ch->transfer_to)
        ch = ch->transfer_to;

    struct event *event = kmalloc(sizeof(*event), KMALLOC_NORMAL);
    event->next    = NULL;
    event->channel = ch;
    event->type    = type;
    event->flags   = 0;
    event->arg     = arg;

    struct listener *listener = kmalloc(sizeof(*listener), KMALLOC_NORMAL);
    listener->next    = NULL;
    listener->channel = ch;
    ch->flags |= CHANNEL_EVENT;

    mutex_lock(&ch->receiver_lock);
    insert_into_list((struct list **) &ch->events, event);
    insert_into_list((struct list **) &listeners, listener);
    mutex_unlock(&ch->receiver_lock);

    return event;
}


bool __fire_event(struct event *e) {
    e->flags |= EVENT_FIRED;

    struct thread *receiver = e->channel->receiver;
    if (receiver)
        resume_thread(receiver);

    DEBUG("fired the event %d.%d (#%d)", e->type >> 12, e->type & 0xfff,
          (receiver) ? receiver->tid : 0);

    return true;
}


bool _fire_event(struct channel *ch, msgid_t type) {

    if (ch->transfer_to)
        ch = ch->transfer_to;

    for (struct event *e = ch->events; e; e = e->next) {
         if (e->type == type) {
             return __fire_event(e);
         }
     }

     return false;
}


void fire_event(msgid_t type) {

    for (struct listener* listener = listeners; listener; listener = listener->next) {
        if (_fire_event(listener->channel, type))
            return;
    }

    DEBUG("no event listener of the event 0x%x", type);
}
