#ifndef __KERNEL_EVENT_H__
#define __KERNEL_EVENT_H__

#include <resea.h>
#include "channel.h"


enum {
    EVENT_FIRED = 1,
};


struct event {
    struct event *next;
    msgid_t type;
    int flags;
    uintmax_t arg;
};


void listen_event(struct channel *ch, msgid_t type, uintmax_t arg);
void fire_event(msgid_t type);

#endif
