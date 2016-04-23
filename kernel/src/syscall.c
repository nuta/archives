#include <resea.h>
#include <string.h>
#include "kernel.h"


static result_t send(channel_t ch, payload_t *m, size_t size) {
    struct thread_group *src_group, *dest_group;
    struct channel *src, *dest;

    src_group = kernel_get_current_thread_group();
    src = &src_group->channels[ch];

    if (!src->used || !src->linked_to) {
       WARN("unconnected channel (%d), aborting", ch);
       return E_UNCONNECTED;
    }

    dest_group = kernel_get_thread_group(src->linked_group);
    dest = &dest_group->channels[src->linked_to];

    if (dest->transfer_to) {
       dest = &dest_group->channels[dest->transfer_to];
    }

    // TODO: session payload
    // TODO: deny empty payloads (require at least one payload header)
    // TODO: validate payloads size

    if (size % sizeof(payload_t) != 0) {
        WARN("message size must be a multiple of sizeof(payload_t)");
        return E_INVALID;
    }

    if (size < sizeof(payload_t)) {
        // we need at least one payload header
        WARN("message is too short");
        return E_INVALID;
    }

    // foreach payload
    payload_t header;
    size_t payloads_num = size / sizeof(payload_t);

    for (int i = 0; i < payloads_num; i++) {
        if (i % 8 == 0) {
            header = m[i];
        } else {
            int type = (header >> (4 * ((i - 1) % 8))) & 0x0f;
         
            switch (type) {
            case PAYLOAD_INLINE:
                break;
            case PAYLOAD_OOL:
                WARN("OoL payload is not supported yet, handling as an inline");
                break;
            case PAYLOAD_MOVE_OOL:
                WARN("move OoL payload is not supported yet, handling as an inline");
                break;
            case PAYLOAD_CHANNEL:
                if (kernel_get_channel(src_group, m[i])->linked_to) {
                    WARN("channel (%d th payload) is already linked", i);
                    return E_INVALID;
                }

                kernel_link_channels(src_group, m[i], dest_group, kernel_alloc_channel_id(dest_group));
                break;
            case PAYLOAD_NULL:
                break;
            default:
                WARN("unknown payload type (%d), handling as an inline",
                     type);
            }
        }
    }

    if (dest_group->id == 1) {
        // destination thread group is in the kernel space
        handler_t *handler = (handler_t *) dest->handler;

        if (handler) {
            INFO("send to handler: @%d:%d -> @%d:%d (handler=%p)",
                 src_group->id, ch, dest_group->id, src->linked_to,
                 handler);
            handler(src->linked_to, m);
        } else {
            INFO("send to queue: @%d:%d -> @%d:%d (queue=%p)",
                 src_group->id, ch, dest_group->id, src->linked_to,
                 dest->buf);
            if (dest->buf == (uintptr_t) NULL) {
                WARN("buffer is not set");
                return E_NOSPACE;
            }

            memcpy((void *) dest->buf, m, size); // XXX
        }

        return OK;
    }

    INFO("unsupported"); // TODO
    return E_NOTSUPPORTED;
}


static result_t wait(channel_t ch) {

    return E_NOTSUPPORTED;
}


static result_t recv(channel_t ch, payload_t **m) {

    /* XXX: kernel_switch_thread() will cause a problem because the stack in
            the system call handler is not for kernel */
    return OK;
}

static result_t setoptions(channel_t channel, handler_t *handler, void *buffer, size_t size) {
    struct thread_group *g;

    g = kernel_get_current_thread_group();
    if (channel > CHANNELS_MAX || !g->channels[channel].used) {
        WARN("invalid channel ID");
        return E_INVALID;
    }

    g->channels[channel].handler = (uintptr_t) handler;
    g->channels[channel].buf     = (uintptr_t) buffer;
    g->channels[channel].buf_num = (size_t) size;
    return OK;
}



static result_t call(channel_t ch, payload_t *m, size_t size, void *buffer, size_t buffer_size) {
    result_t r;

    if ((r = setoptions(ch, NULL, buffer, buffer_size)) != OK)
        return r;

    if ((r = send(ch, m, size)) != OK)
        return r;

    return wait(ch);
}

static result_t link(channel_t ch1, channel_t ch2) {
    struct thread_group *g;

    g = kernel_get_current_thread_group();
    if (ch1 > CHANNELS_MAX || !g->channels[ch1].used) {
        WARN("invalid channel ID");
        return E_INVALID;
    }

    if (ch2 > CHANNELS_MAX || !g->channels[ch2].used) {
        WARN("invalid channel ID");
        return E_INVALID;
    }

    kernel_link_channels(g, ch1, g, ch2);
    return OK;
}


static result_t transfer(channel_t ch1, channel_t ch2) {
    struct thread_group *g;

    g = kernel_get_current_thread_group();
    if (ch1 > CHANNELS_MAX || !g->channels[ch1].used) {
        WARN("invalid channel ID");
        return E_INVALID;
    }

    if (ch2 > CHANNELS_MAX || !g->channels[ch2].used) {
        WARN("invalid channel ID");
        return E_INVALID;
    }

    kernel_transfer_to(g, ch1, ch2);
    return OK;
}


static result_t open(channel_t *ch) {

    *ch = kernel_alloc_channel_id(kernel_get_current_thread_group());
    return OK;
}


static result_t close(channel_t id) {

    WARN("sys_close");
    return OK;
}

// NOTE: If you want to change this defition do care about
//       system call wrappers in lang libraries.
result_t kernel_syscall(int type, uintmax_t r1, uintmax_t r2,
                        uintmax_t r3, uintmax_t r4, uintmax_t r5, uintmax_t *ret) {
    result_t r;

    switch (type) {
    case SYSCALL_OPEN:
        DEBUG("syscall: open()");
        r = open((channel_t *) ret);
        break;
    case SYSCALL_CLOSE:
        DEBUG("syscall: close(%d)", r1);
        r = close((channel_t) r1);
        break;
    case SYSCALL_SETOPTIONS:
        DEBUG("syscall: setoptions(%d, %p, %p, %d)", r1, r2, r4, r4);
        r = setoptions((channel_t) r1, (handler_t *) r2, (void *) r3, (size_t) r4);
        break;
    case SYSCALL_SEND:
        DEBUG("syscall: send(%p)", (void *) r1);
        r = send((channel_t) r1, (payload_t *) r2, (size_t) r3);
        break;
    case SYSCALL_RECV:
        DEBUG("syscall: recv(%d)", r1);
        r = recv((channel_t) r1, (payload_t **) &ret);
        break;
    case SYSCALL_WAIT:
        DEBUG("syscall: wait(%d)", r1);
        r = wait((channel_t) r1);
        break;
     case SYSCALL_CALL:
        DEBUG("syscall: call(%p, %p)", (void *) r1, (void *) r2);
        r = call((channel_t) r1, (payload_t *) r2, (size_t) r3, (void *) r4, (size_t) r5);
        break;
    case SYSCALL_LINK:
        DEBUG("syscall: link(%d, %d)", r1, r2);
        r = link((channel_t) r1, (channel_t) r2);
        break;
    case SYSCALL_TRANSFER:
        DEBUG("syscall: transfer(%d, %d)", r1, r2);
        r = transfer((channel_t) r1, (channel_t) r2);
        break;
    default:
        WARN("syscall: unknown type: %d", type);
        r = E_INVALID;
    }

    return r;
}


