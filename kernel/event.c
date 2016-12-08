#include "kmalloc.h"
#include "thread.h"
#include "channel.h"
#include "event.h"
#include "list.h"


static struct channel *listeners = NULL;

struct event *listen_event(struct channel *ch, msgid_t type, uintmax_t arg) {

    if (ch->transfer_to)
        ch = ch->transfer_to;

    struct event *new_e = kmalloc(sizeof(*new_e), KMALLOC_NORMAL);
    new_e->next    = NULL;
    new_e->channel = ch;
    new_e->type    = type;
    new_e->flags   = 0;
    new_e->arg     = arg;

    ch->flags |= CHANNEL_EVENT;

    mutex_lock(&ch->receiver_lock);
    insert_into_list((struct list **) &ch->events, new_e);
    insert_into_list((struct list **) &listeners, ch);
    mutex_unlock(&ch->receiver_lock);

    return new_e;
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
             __fire_event(e);
         }
     }

     return false;
}


void fire_event(msgid_t type) {

    for (struct channel * ch = listeners; ch; ch = ch->next) {
        if (_fire_event(ch, type))
            return;
    }

    DEBUG("no event listener of the event 0x%x", type);
}
