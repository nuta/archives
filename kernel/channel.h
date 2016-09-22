#ifndef __KERNEL_CHANNEL_H__
#define __KERNEL_CHANNEL_H__

#include <mutex.h>
#include <queue.h>

enum {
    CHANNEL_OPEN,      // created, but not linked
    CHANNEL_LINKED,    // ready for communication
    CHANNEL_CLOSED,    // closed by the our process
    CHANNEL_UNLINKED,  // closed by the destination process
};

struct process;
struct channel {
    int state;
    cid_t cid;
    mutex_t sender_lock;
    mutex_t receiver_lock;
    uintptr_t buffer;
    size_t buffer_size;
    cid_t sent_from;
    struct channel *linked_to;
    struct channel *transfer_to;
    struct process *process;    // Owner process
    struct thread *sender;      // Sender thread
    struct thread *receiver;    // Receiver thread
};

#endif
