#include <mutex.h>
#include "channel.h"
#include "message.h"
#include "thread.h"
#include "kmalloc.h"
#include "list.h"


static cid_t last_cid = 0;
static mutex_t allocate_cid_lock = MUTEX_INITIALIZER;

static cid_t allocate_cid(void) {
    cid_t cid;

    // TODO: handle overflow and duplication
    mutex_lock(&allocate_cid_lock);
    last_cid++;
    cid = last_cid;
    mutex_unlock(&allocate_cid_lock);

    return cid;
}


cid_t _open(struct process *proc) {

    mutex_lock(&proc->channels_lock);

    // TODO: add kmalloc option to raise PANIC if it run out of memory
    struct channel *ch = (struct channel *) kmalloc(sizeof(*ch),
                                                    KMALLOC_NORMAL);

    // Initialize the allocated channel
    ch->flags       = CHANNEL_OPEN;
    ch->cid         = allocate_cid();
    ch->process     = proc;
    ch->sent_from   = 0;
    ch->events      = NULL;
    ch->sender      = NULL;
    ch->receiver    = NULL;
    ch->linked_to   = NULL;
    ch->transfer_to = NULL;
    ch->next        = NULL;
    mutex_init(&ch->sender_lock);
    mutex_init(&ch->receiver_lock);

    insert_into_list((struct list **) &proc->channels, ch);
    mutex_unlock(&proc->channels_lock);
    return ch->cid;
}


cid_t open(void) {

    return _open(get_current_thread()->process);
}
