#include <resea.h>
#include <mutex.h>
#include <string.h>
#include "channel.h"
#include "message.h"
#include "thread.h"
#include "kmalloc.h"


result_t _send(struct channel *ch, const void *m, size_t size, int flags) {

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


result_t send(cid_t cid, const void *m, size_t size, int flags) {

    struct channel *ch = get_channel_by_cid(cid);
    if (!ch) {
        return E_BAD_CID;
    }

    return _send(ch, m, size, flags);
}
