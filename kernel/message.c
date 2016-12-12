#include <arch.h>
#include <mutex.h>
#include <string.h>
#include <logging.h>
#include "event.h"
#include "message.h"
#include "kmalloc.h"
#include "channel.h"
#include "process.h"
#include "thread.h"
#include "panic.h"
#include "list.h"


struct channel *_get_channel_by_cid(struct process *proc, cid_t cid) {

    for (struct channel *ch = proc->channels; ch; ch = ch->next) {
        if (ch->cid == cid)
            return ch;
    }

    PANIC("bad cid %d",cid);
}


struct channel *get_channel_by_cid(cid_t cid) {

    return _get_channel_by_cid(get_current_thread()->process, cid);
}


static result_t _send(struct channel *ch, const void *m, size_t size, int flags) {

    if (CHANNEL_STATE(ch) != CHANNEL_LINKED) {
        WARN("bad channel (@%d)", ch->cid);
        return E_BAD_CID;
    }

    struct channel *dst = ch->linked_to;
    cid_t sent_from = dst->cid;
    if (dst->transfer_to) {
        dst = dst->transfer_to;
    }

    // Acquire a sender right
    while (!mutex_try_lock(&dst->sender_lock)) {
        // TODO: block
        yield();
    }

    // Wait for the receiver
    struct thread *current = get_current_thread();
    set_thread_state(current, THREAD_BLOCKED);
    dst->sender = current;
    while (!dst->receiver || dst->sent_from != 0) {
        // Yield so that another thread can receive the channel. The current
        // thread is blocked until the receiver resumes it.
        yield();
    }

    // Parse payload headers and transform payloads.
    payload_t *payloads = (payload_t *) m;
    size_t payloads_num = size / sizeof(payload_t);
    payload_t header = 0;
    payload_t pointer = 0;
    int prev_type = 0;

    for (int i=0; i < payloads_num; i++) {
        if (IS_PAYLOAD_HEADER_INDEX(i)) {
            header = payloads[i];
        } else {
            int type = GET_PAYLOAD_TYPE(header, i);

            switch (type) {
            case PAYLOAD_NULL:
                break;
            case PAYLOAD_INLINE:
                // Nothing to do
                break;
            case PAYLOAD_POINTER:
                pointer = payloads[i];
                break;
            case PAYLOAD_POINTER_SIZE:
                if (prev_type != PAYLOAD_POINTER) {
                    WARN("unexpected pointer size payload");
                    break;
                }

                if (!payloads[i] || !pointer) {
                    payloads[i - 1] = 0;
                    continue;
                }

                size_t pointer_size = payloads[i];
                if (current->process->pid == 1 && current->process == dst->process) {
                    // in-kernel
                    void *new_pointer = kmalloc(pointer_size, KMALLOC_NORMAL);
                    memcpy_s(new_pointer, pointer_size, (const void *) pointer, pointer_size);
                    payloads[i - 1] = (payload_t) new_pointer;
                } else {
                    NOT_YET_IMPLEMENTED();
                }

                break;
            case PAYLOAD_CHANNEL: {
                cid_t cid = _open(dst->process);
                struct channel *receiver_ch = _get_channel_by_cid(dst->process, cid);
                struct channel *sender_ch = get_channel_by_cid(payloads[i]);

                if (CHANNEL_STATE(sender_ch) == CHANNEL_LINKED) {
                    NOT_YET_IMPLEMENTED();
                    payloads[i] = 0;
                    break;
                }

                _link(sender_ch, receiver_ch);
                payloads[i] = receiver_ch->cid;
                break;
            }
            default:
                // Handle an unknown payload as inline.
                WARN("unknown payload type: %d", type);
            }

            prev_type = type;
        }
    }

    // Copy the message.
    // TODO: access right check
    if (memcpy_s((void *) dst->buffer, dst->buffer_size, m, size) != OK) {
        WARN("failed to copy the message");
        return E_BAD_MEMCPY;
    }

    dst->sent_from = sent_from;
    resume_thread(dst->receiver);
    mutex_unlock(&dst->sender_lock);

    return OK;
}


static result_t _recv(struct channel *ch, void *buffer, size_t size, int flags,
                      cid_t *from) {

    struct thread *current = get_current_thread();

retry:
    set_thread_state(current, THREAD_BLOCKED);
    ch->receiver    = current;
    ch->buffer      = (uintptr_t) buffer;
    ch->buffer_size = size;

    if (ch->flags & CHANNEL_EVENT && mutex_try_lock(&ch->sender_lock)) {
        // Look for a fired event.
        for (struct event *e = ch->events; e; e = e->next) {
            if (e->flags & EVENT_FIRED) {
                // Found a fired event.
                e->flags &= ~EVENT_FIRED;

                payload_t m[3];
                m[0] = (1 << 4) | 1;
                m[1] = e->type;
                m[2] = e->arg;

                if (memcpy_s((void *) ch->buffer, ch->buffer_size, &m, sizeof(m)) != OK) {
                    WARN("failed to fill an event message");
                    return E_BAD_MEMCPY;
                }

                ch->sent_from = -1;
                mutex_unlock(&ch->sender_lock);
                goto received;
            }
        }

        mutex_unlock(&ch->sender_lock);
    }

    // Resume a sender thread if it exists.
    struct thread *sender = ch->sender;
    if (sender)
        resume_thread(sender);

    // Yield so that the sender thread can send a message.
    if (current->state == THREAD_BLOCKED)
        yield();

    if (!ch->sent_from) {
        // Resumed by an event message.
        goto retry;
    }

    // Sending is done and the sent message is filled in `buffer`.
received:
    *from = ch->sent_from;
    ch->sent_from = 0;
    ch->receiver = NULL;
    mutex_unlock(&ch->receiver_lock); // locked in recv()
    return OK;
}


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


result_t send(cid_t cid, const void *m, size_t size, int flags) {

    struct channel *ch = get_channel_by_cid(cid);
    if (!ch) {
        return E_BAD_CID;
    }

    return _send(ch, m, size, flags);
}


result_t recv(cid_t cid, void *buffer, size_t size, int flags, cid_t *from) {

    struct channel *ch = get_channel_by_cid(cid);

    if (!ch) {
        return E_BAD_CID;
    }

    if (!mutex_try_lock(&ch->receiver_lock)) {
        return E_ALREADY_RECVED;
    }

    return _recv(ch, buffer, size, flags, from);
}


result_t call(cid_t cid, const void *m, size_t m_size,
              void *r, size_t r_size) {

    result_t result;

    struct channel *ch = get_channel_by_cid(cid);
    if (!ch) {
        return E_BAD_CID;
    }

    // Lock the receiver lock of `src` channel to prevent
    // other threads from receiving the reply message of this
    // message.
    mutex_lock(&ch->receiver_lock);

    result = _send(ch, m, m_size, 0);
    if (result != OK) {
        return result;
    }

    cid_t from; // We don't care this.
    return _recv(ch, r, r_size, 0, &from);
}


static inline void update_channel_state(struct channel *ch, int state) {

    ch->flags = (ch->flags & (~3)) | state;
}


result_t _link(struct channel *ch1, struct channel *ch2) {
    // TODO: add a assertion

    ch1->linked_to = ch2;
    ch2->linked_to = ch1;
    update_channel_state(ch1, CHANNEL_LINKED);
    update_channel_state(ch2, CHANNEL_LINKED);

    return OK;
}


result_t link(cid_t cid1, cid_t cid2) {

    struct channel *ch1 = get_channel_by_cid(cid1);
    struct channel *ch2 = get_channel_by_cid(cid2);

    if (!ch1 || !ch2) {
        return E_BAD_CID;
    }

    return _link(ch1, ch2);
}


result_t _transfer(struct channel *from, struct channel *to) {
    // TODO: add a assertion

    from->transfer_to = to;
    return OK;
}


result_t transfer(cid_t from, cid_t to) {

    struct channel *_from = get_channel_by_cid(from);
    struct channel *_to   = get_channel_by_cid(to);

    if (!_from || !_to) {
        return E_BAD_CID;
    }

    return _transfer(_from, _to);
}


result_t discard(void *buffer, size_t size) {

    payload_t *payloads = (payload_t *) buffer;
    size_t payloads_num = size / sizeof(payload_t);
    payload_t header = 0;
    for (int i=0; i < payloads_num; i++) {
        if (IS_PAYLOAD_HEADER_INDEX(i)) {
            header = payloads[i];
        } else {
            int type = GET_PAYLOAD_TYPE(header, i);

            switch (type) {
            case PAYLOAD_POINTER:
                try_kfree((void *) payloads[i]);
                break;
            }
        }
    }

    return OK;
}
