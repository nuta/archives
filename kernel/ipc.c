#include "cpu.h" // FIXME
#include "printk.h"
#include "thread.h"
#include "process.h"
#include "ipc.h"

static inline void link_channels(struct channel *ch1, struct channel *ch2) {
    ch1->linked_to = ch2;
    ch2->linked_to = ch1;
}


static inline void transfer_to(struct channel *from, struct channel *to) {
    from->transfer_to = to;
}


static inline struct channel *get_channel_by_id(channel_t cid) {
    size_t channels_max = CPUVAR->current->process->channels_max;
    struct channel *channels = (struct channel *) &CPUVAR->current->process->channels;

    if (cid == 0 || cid > channels_max) {
        return NULL;
    }

    struct channel *ch = &channels[cid - 1];
    return (ch->flags == 0) ? NULL : ch;
}

struct channel *channel_create(struct process *process) {
    kmutex_state_t state = kmutex_lock_irq_disabled(&process->lock);
    size_t channels_max = process->channels_max;
    for (size_t i = 0; i < channels_max; i++) {
        struct channel *ch = &process->channels[i];
        if (ch->flags == 0) {
            ch->flags = 1;
            ch->cid = i + 1;
            ch->process = process;
            ch->linked_to = NULL;
            ch->transfer_to = NULL;
            ch->receiver = NULL;
            ch->sender = NULL;
            waitqueue_list_init(&ch->wq);
            kmutex_unlock_restore_irq(&process->lock, state);
            return ch;
        }
    }

    kmutex_unlock_restore_irq(&process->lock, state);
    return NULL;
}


channel_t channel_connect(struct channel *server, struct process *client) {
    struct channel *server_side = channel_create(server->process);
    struct channel *client_side = channel_create(client);
    // TODO: error check
    link_channels(server_side, client_side);
    transfer_to(server_side, server);

    DEBUG("sys.connect: @%d.%d -> @%d.%d ~> @%d.%d",
        client_side->process->pid, client_side->cid,
        server_side->process->pid, server_side->cid,
        server->process->pid, server->cid);

    return client_side->cid;
}


static inline void close_channel(struct channel *ch) {
    ch->flags = 0;
}


static payload_t copy_payload(
    int type,
    struct process *src,
    struct process *dst,
    payload_t payload,
    size_t ool_length
) {

    switch (type) {
        case PAYLOAD_INLINE:
            return payload;
        case PAYLOAD_OOL: {
            if (dst == kernel_process) {
                void *kv = kmalloc(ool_length, KMALLOC_NORMAL);
                arch_copy_from_user(kv, payload, ool_length);
                return (payload_t) kv;
            } else {
                size_t allocated_size = ROUND_UP(ool_length, PAGE_SIZE);
                size_t pages_num = allocated_size / PAGE_SIZE;
                paddr_t p = alloc_pages(pages_num, KMALLOC_NORMAL);
                void *kv = from_paddr(p);
                uptr_t v = valloc(&dst->vms, allocated_size);
                int attrs = PAGE_USER | PAGE_WRITABLE;
                arch_copy_from_user(kv, payload, ool_length);
                arch_link_page(&dst->vms.arch, v, p, pages_num, attrs);
                return v;
            }
        }
        case PAYLOAD_CHANNEL: {
            struct channel *ch = get_channel_by_id(payload);
            if (!ch) {
                DEBUG("copy_payload: invalid ch %d", payload);
                return 0; // TODO: return error
            }

            struct channel *new_ch = channel_create(dst);
            if (!new_ch) {
                DEBUG("copy_payload: failed to create channel");
                return 0;
            }

            if (ch->linked_to) {
                DEBUG("copy_payload: link @%d.%d <-> @%d.%d",
                    ch->linked_to->process->pid, ch->linked_to->cid,
                    new_ch->process->pid, new_ch->cid);
                ch->linked_to->linked_to = new_ch;
                new_ch->linked_to = ch->linked_to;
                close_channel(ch);
            } else {
                DEBUG("copy_payload: link @%d.%d <-> @%d.%d",
                    ch->process->pid, ch->cid,
                    new_ch->process->pid, new_ch->cid);
                new_ch->linked_to = ch;
                ch->linked_to = new_ch;
            }

            return new_ch->cid;
        }
    }

    // Invalid payload type.
    return 0;
}

channel_t sys_open(void) {
    struct channel *ch = channel_create(CPUVAR->current->process);
    if(!ch) {
        DEBUG("sys_open: failed to allocate #%d", CPUVAR->current->process->pid);
        return ERROR_NO_MEMORY;
    }

    DEBUG("sys_open: #%d allocate @%d", CPUVAR->current->process->pid, ch->cid);
    return ch->cid;
}


header_t sys_call(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3,
    payload_t *rs
) {
    header_t error = sys_send(ch, type, a0, a1, a2, a3);
    if (error != ERROR_NONE) {
        return error;
    }

    return sys_recv(ch, rs);
}


header_t sys_replyrecv(
    channel_t client,
    header_t type,
    payload_t r0,
    payload_t r1,
    payload_t r2,
    payload_t r3,
    payload_t *rs
) {
    header_t error = sys_send(client, type, r0, r1, r2, r3);
    if (error != ERROR_NONE) {
        return error;
    }

    struct channel *server = get_channel_by_id(client)->transfer_to;
    if (!server) {
        return ERROR_CH_NOT_TRANSFERED;
    }

    return sys_recv(server->cid, rs);
}


header_t sys_send(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3
) {
    struct channel *src = get_channel_by_id(ch);
    if (!src) {
        DEBUG("sys_send: @%d no such channel", ch);
        return ERROR_INVALID_CH;
    }

    struct channel *linked_to = src->linked_to;
    if (!linked_to) {
        DEBUG("sys_send: @%d not linked", ch);
        return ERROR_CH_NOT_LINKED;
    }

    struct thread *current_thread = CPUVAR->current;
    struct channel *dst = (linked_to->transfer_to) ? linked_to->transfer_to : linked_to;

    DEBUG("sys_send: @%d.%d -> @%d.%d ~> @%d.%d (type=%d.%d)",
        src->process->pid, src->cid,
        linked_to->process->pid, linked_to->cid,
        dst->process->pid, dst->cid,
        MSG_SERVICE_ID(type), MSG_ID(type));

    // Get the sender right.
    while (true) {
        if (atomic_compare_and_swap(&dst->sender, NULL, current_thread)) {
            // Now we have a sender right (dst->sender == current_thread).
            break;
        }

        // XXX: lock the wait queue or current thread will never wake up

        // Another thread is sending to the destination. Add current thread
        // to wait queue. A receiver thread will resume it.
        struct waitqueue *wq = kmalloc(sizeof(*wq), KMALLOC_NORMAL);
        wq->thread = current_thread;

        waitqueue_list_append(&dst->wq, wq);
        thread_block_current();
    }

    // Wait for the receiver.
    if (!dst->receiver) {
        thread_block_current();
    }

    // Copy payloads.
    struct process *src_process = CPUVAR->current->process;
    struct process *dst_process = dst->process;
    dst->header = type;
    dst->sent_from = linked_to->cid;
    dst->buffer[0] = copy_payload(PAYLOAD_TYPE(type, 0), src_process, dst_process, a0, a1);
    dst->buffer[1] = copy_payload(PAYLOAD_TYPE(type, 1), src_process, dst_process, a1, a2);
    dst->buffer[2] = copy_payload(PAYLOAD_TYPE(type, 2), src_process, dst_process, a2, a3);
    dst->buffer[3] = copy_payload(PAYLOAD_TYPE(type, 3), src_process, dst_process, a3, 0);

    thread_resume(dst->receiver);
    return ERROR_NONE;
}


header_t sys_recv(
    channel_t ch,
    payload_t *rs
) {
    struct channel *src = get_channel_by_id(ch);
    if (!src) {
        DEBUG("sys_recv: @%d no such channel", ch);
        return ERROR_INVALID_CH;
    }

    // Try to get the receiver right.
    struct thread *current_thread = CPUVAR->current;
    if (!atomic_compare_and_swap(&src->receiver, NULL, current_thread)) {
        return ERROR_CH_IN_USE;
    }

    // Wait for the sender.
    // Resume a thread in the wait queue.
    if (src->sender) {
        // The sender waits for us. Resume it and wait for it.
        thread_resume(src->sender);
    } else {
        struct waitqueue *wq = waitqueue_list_pop(&src->wq);
        if (wq) {
            thread_resume(wq->thread);
            kfree(wq);
        } else {
        }
    }

    thread_block_current();

    // Receiver sent a reply message and resumed the sender thread. Do recv
    // work.
    header_t reply_header = src->header;
    rs[0] = src->sent_from;
    rs[1] = src->buffer[0];
    rs[2] = src->buffer[1];
    rs[3] = src->buffer[2];
    rs[4] = src->buffer[3];
    src->receiver = NULL;
    src->sender = NULL;
    return reply_header;
}


channel_t sys_connect(channel_t server) {
    struct channel *ch = get_channel_by_id(server);
    if (!ch) {
        DEBUG("sys_connect: @%d no such channel", server);
        return ERROR_INVALID_CH;
    }

    struct channel *linked_to = ch->linked_to;
    if (!linked_to) {
        DEBUG("sys_connect: @%d not linked", ch);
        return ERROR_CH_NOT_LINKED;
    }

    return channel_connect(linked_to, CPUVAR->current->process);
}


channel_t sys_link(channel_t ch1, channel_t ch2) {
    /* TODO */
}


channel_t sys_transfer(channel_t ch, channel_t dest) {
    /* TODO */
}


channel_t sys_discard(payload_t ool0, payload_t ool1, payload_t ool2, payload_t ool3) {
    /* TODO */
}


header_t invalid_syscall(void) {
    INFO("invalid syscall");
    return 0;
}
