#ifndef __KERNEL_CHANNEL_H__
#define __KERNEL_CHANNEL_H__

#include <mutex.h>

enum {
    CHANNEL_OPEN     = 0,  // created, but not linked
    CHANNEL_LINKED   = 1,  // ready for communication
    CHANNEL_CLOSED   = 2,  // closed by the our process
    CHANNEL_UNLINKED = 3,  // closed by the destination process
    CHANNEL_EVENT    = (1 << 2),
};

#define CHANNEL_STATE(ch)  ((ch)->flags & 3)

struct event;
struct process;

struct channel {
    struct channel *next;
    int flags;
    cid_t cid;
    mutex_t sender_lock;
    mutex_t receiver_lock;
    uintptr_t buffer;
    size_t buffer_size;
    cid_t sent_from;
    struct event *events;
    struct channel *linked_to;
    struct channel *transfer_to;
    struct process *process;    // Owner process
    struct thread *sender;      // Sender thread
    struct thread *receiver;    // Receiver thread
};

struct channel *get_channel_by_cid(cid_t cid);

#endif
