#include <resea.h>
#include <mutex.h>
#include <string.h>
#include "channel.h"
#include "event.h"
#include "message.h"
#include "thread.h"
#include "kmalloc.h"


result_t _recv(struct channel *ch, void *buffer, size_t size, int flags,
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
