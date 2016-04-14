#include <resea.h>
#include <string.h>
#include "kernel.h"


result_t sys_send(channel_t ch, payload_t *m) {
    struct thread_group *src_group, *dest_group;
    struct channel *src, *dest;

    src_group = kernel_get_current_thread_group();
    src = &src_group->channels[ch];

    if (!src->used || !src->dest_ch) {
       WARN("unconnected channel (%d), aborting", ch);
       return E_UNCONNECTED;
    }

    dest_group = kernel_get_thread_group(src->dest);
    dest = &dest_group->channels[src->dest_ch];
    if (dest->transfer_to) {
       dest = &dest_group->channels[dest->transfer_to];
    }

    // TODO: channel payload
    // TODO: session payload
    // TODO: deny empty payloads (require at least one payload header)
    // TODO: validate payloads size

    if (dest_group->id == 1) {
        // destination thread group is in the kernel space
        handler_t *handler = (handler_t *) dest->handler;

        if (handler) {
            INFO("send to handler: @%d:%d -> @%d:%d (handler=%p)",
                 src_group->id, ch, dest_group->id, src->dest_ch,
                 handler);
            handler(src->dest_ch, m);
        } else {
            size_t size = 64; // XXX
            INFO("send to queue: @%d:%d -> @%d:%d (queue=%p)",
                 src_group->id, ch, dest_group->id, src->dest_ch,
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


result_t sys_wait(channel_t ch) {

    return E_NOTSUPPORTED;
}


result_t sys_recv(channel_t ch, payload_t **m) {

    /* XXX: kernel_switch_thread() will cause a problem because the stack in
            the system call handler is not for kernel */
    return OK;
}


result_t sys_call(channel_t ch, payload_t *m, void *buffer, size_t buffer_size) {
    result_t r;

    if ((r = sys_setoptions(ch, NULL, buffer, buffer_size)) != OK)
        return r;

    if ((r = sys_send(ch, m)) != OK)
        return r;

    return sys_wait(ch);
}


result_t sys_setoptions(channel_t channel, handler_t *handler, void *buffer, size_t size) {
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


result_t sys_link(channel_t ch1, channel_t ch2) {
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

    kernel_connect_channels(g, ch1, g, ch2);
    return OK;
}


result_t sys_transfer(channel_t ch1, channel_t ch2) {
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


result_t sys_open(channel_t *ch) {

    *ch = kernel_alloc_channel_id(kernel_get_current_thread_group());
    return OK;
}


result_t sys_close(channel_t id) {

    WARN("sys_close");
    return OK;
}

// TODO: transfer(ch_from, ch_to)

result_t kernel_syscall(enum SyscallType type, uintmax_t r1, uintmax_t r2, uintmax_t r3,
                      uintmax_t r4, uintmax_t *ret){
    result_t r;

    switch (type) {
    case SYSCALL_OPEN:
        DEBUG("syscall: open()");
        r = sys_open((channel_t *) ret);
        break;
    case SYSCALL_CLOSE:
        DEBUG("syscall: close(%d)", r1);
        r = sys_close((channel_t) r1);
        break;
    case SYSCALL_SETOPTIONS:
        DEBUG("syscall: setoptions(%d, %p, %p, %d)", r1, r2, r4, r4);
        r = sys_setoptions((channel_t) r1, (handler_t *) r2, (void *) r3, (size_t) r4);
        break;
    case SYSCALL_SEND:
        DEBUG("syscall: send(%p)", (void *) r1);
        r = sys_send((channel_t) r1, (payload_t *) r2);
        break;
    case SYSCALL_RECV:
        DEBUG("syscall: recv(%d)", r1);
        r = sys_recv((channel_t) r1, (payload_t **) &ret);
        break;
    case SYSCALL_WAIT:
        DEBUG("syscall: wait(%d)", r1);
        r = sys_wait((channel_t) r1);
        break;
     case SYSCALL_CALL:
        DEBUG("syscall: call(%p, %p)", (void *) r1, (void *) r2);
        r = sys_call((channel_t) r1, (payload_t *) r2, (void *) r3, (size_t) r4);
        break;
    case SYSCALL_LINK:
        DEBUG("syscall: link(%d, %d)", r1, r2);
        r = sys_link((channel_t) r1, (channel_t) r2);
        break;
    case SYSCALL_TRANSFER:
        DEBUG("syscall: transfer(%d, %d)", r1, r2);
        r = sys_transfer((channel_t) r1, (channel_t) r2);
        break;
    default:
        WARN("syscall: unknown type: %d", type);
        r = E_INVALID;
    }

    return r;
}


