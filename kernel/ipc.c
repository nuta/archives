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

static struct channel *get_channel_by_id_of(struct process *proc, channel_t cid) {
    size_t channels_max = proc->channels_max;
    struct channel *channels = (struct channel *) &proc->channels;

    if (cid == 0 || cid > channels_max) {
        return NULL;
    }

    struct channel *ch = &channels[cid - 1];
    return (ch->flags == 0) ? NULL : ch;
}

struct channel *get_channel_by_id(channel_t cid) {
    return get_channel_by_id_of(CPUVAR->current->process, cid);
}

struct channel *channel_create(struct process *process) {
    kmutex_state_t state = kmutex_lock_irq_disabled(&process->lock);
    size_t channels_max = process->channels_max;
    for (size_t i = 0; i < channels_max; i++) {
        struct channel *ch = &process->channels[i];
        if (ch->flags == 0) {
            ch->flags = 1;
            ch->notifications = 0;
            ch->cid = i + 1;
            ch->process = process;
            ch->linked_to = NULL;
            ch->transfer_to = ch;
            ch->receiver = NULL;
            ch->sender = NULL;
            waitqueue_list_init(&ch->wq);
            kmutex_init(&ch->lock, KMUTEX_UNLOCKED);
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


static payload_t copy_payload(int type, struct process *src, struct process *dst,
                              payload_t payload, size_t ool_length) {

    switch (type) {
        case PAYLOAD_INLINE:
            return payload;
        case PAYLOAD_OOL: {
            if (dst == kernel_process) {
                void *kv = kmalloc(ool_length, KMALLOC_NORMAL);
                arch_copy_from_user(kv, payload, ool_length);
                return (payload_t) kv;
            } else {
                size_t pages_num = LEN_TO_PAGE_NUM(ool_length);
                paddr_t p = alloc_pages(pages_num, KMALLOC_NORMAL);
                void *kv = from_paddr(p);
                uptr_t v = valloc(&dst->vms, pages_num);
                if (unlikely(!v)) {
                    WARN("valloc returned 0");
                    return 0; // TODO: return error
                }

                int attrs = PAGE_USER | PAGE_WRITABLE;
                arch_copy_from_user(kv, payload, ool_length);
                arch_link_page(&dst->vms.arch, v, p, pages_num, attrs);
                DEBUG("copy_payload: ool %p -> %p (len=%d)", payload, v, ool_length);
                return v;
            }
        }
        case PAYLOAD_CHANNEL: {
            struct channel *ch = get_channel_by_id(payload);
            if (!ch) {
                WARN("copy_payload: invalid ch %d", payload);
                return 0; // TODO: return error
            }

            struct channel *new_ch = channel_create(dst);
            if (!new_ch) {
                WARN("copy_payload: failed to create channel");
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

    // Invalid payload header.
    return 0;
}

channel_t sys_open(void) {
    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *ch = channel_create(CPUVAR->current->process);
    if(!ch) {
        WARN("sys_open: failed to allocate #%d", CPUVAR->current->process->pid);
        return ERROR_NO_MEMORY;
    }

    DEBUG("sys_open: #%d allocate @%d", CPUVAR->current->process->pid, ch->cid);
    channel_t cid = ch->cid;

    restore_irq(&irqstate);
    return cid;
}


void sys_close(channel_t ch) {
}


struct msg *sys_call(channel_t ch, header_t header, payload_t a0,
                     payload_t a1, payload_t a2, payload_t a3) {

    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    header_t error = sys_send(ch, header, a0, a1, a2, a3);
    if (unlikely(error != ERROR_NONE)) {
        restore_irq(&irqstate);
        return ERROR_PTR(error);
    }

    restore_irq(&irqstate);
    return sys_recv(ch);
}


struct msg *sys_replyrecv(channel_t client, header_t header, payload_t r0,
                          payload_t r1, payload_t r2, payload_t r3) {

    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    header_t error = sys_send(client, header, r0, r1, r2, r3);
    if (unlikely(error != ERROR_NONE)) {
        restore_irq(&irqstate);
        return ERROR_PTR(error);
    }

    struct channel *server = get_channel_by_id(client)->transfer_to;
    if (unlikely(!server)) {
        restore_irq(&irqstate);
        return ERROR_PTR(ERROR_CH_NOT_TRANSFERED);
    }

    struct msg *m = sys_recv(server->cid);
    restore_irq(&irqstate);
    return m;
}


static header_t sys_send_slowpath(struct channel *src, struct channel *linked_to, header_t header, payload_t a0,
                                  payload_t a1, payload_t a2, payload_t a3) {

    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *dst = linked_to->transfer_to;
    struct thread *current_thread = CPUVAR->current;

    DEBUG("sys_send: @%d.%d -> @%d.%d ~> @%d.%d (header=%d.%d)",
        src->process->pid, src->cid,
        linked_to->process->pid, linked_to->cid,
        dst->process->pid, dst->cid,
        IFTYPE(header), METHODTYPE(header));

    // Get the sender right.
    while (true) {
        if (likely(dst->sender == current_thread)) {
            // We've already got a sender right in sys_send.
            break;
        }

        if (likely(atomic_compare_and_swap(&dst->sender, NULL, current_thread))) {
            // Now we have a sender right (dst->sender == current_thread).
            break;
        }

        // Another thread is sending to the destination. Add current thread
        // to wait queue. A receiver thread will resume it.
        kmutex_state_t irq_state = kmutex_lock_irq_disabled(&dst->lock);
        waitqueue_list_append(&dst->wq, &current_thread->wq);
        kmutex_unlock_restore_irq(&dst->lock, irq_state);
        current_thread->sending = dst;
        thread_block_current();
    }

    // Wait for the receiver.
    if (likely(!dst->receiver)) {
        current_thread->sending = dst;
        thread_block_current();
    }

    // Copy payloads.
    struct process *src_process = CPUVAR->current->process;
    struct process *dst_process = dst->process;
    struct thread *receiver = dst->receiver;
    receiver->buffer.header = header;
    receiver->buffer.sent_from = linked_to->cid;
    receiver->buffer.payloads[0] = copy_payload(PAYLOAD_TYPE(header, 0), src_process, dst_process, a0, a1);
    receiver->buffer.payloads[1] = copy_payload(PAYLOAD_TYPE(header, 1), src_process, dst_process, a1, a2);
    receiver->buffer.payloads[2] = copy_payload(PAYLOAD_TYPE(header, 2), src_process, dst_process, a2, a3);
    receiver->buffer.payloads[3] = copy_payload(PAYLOAD_TYPE(header, 3), src_process, dst_process, a3, 0);

    thread_resume(dst->receiver);
    current_thread->sending = NULL;
    restore_irq(&irqstate);
    return ERROR_NONE;
}


header_t sys_send(channel_t ch, header_t header, payload_t a0, payload_t a1,
                  payload_t a2, payload_t a3) {

    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *src = get_channel_by_id(ch);
    if (unlikely(!src)) {
        restore_irq(&irqstate);
        return ERROR_INVALID_CH;
    }

    struct channel *linked_to = src->linked_to;
    if (unlikely(!linked_to)) {
        restore_irq(&irqstate);
        return ERROR_CH_NOT_LINKED;
    }

    /* Are all payloads inlined? */
    if (unlikely(PAYLOAD_TYPES(header) != 0)) {
        goto slowpath;
    }

    struct channel *dst = linked_to->transfer_to;
    if (unlikely(!atomic_compare_and_swap(&dst->sender, NULL, CPUVAR->current))) {
        goto slowpath;
    }

    struct thread *receiver = dst->receiver;
    if (unlikely(!receiver)) {
        goto slowpath;
    }

    DEBUG("sys_fast_send: @%d.%d -> @%d.%d ~> @%d.%d (header=%d.%d)",
        src->process->pid, src->cid,
        linked_to->process->pid, linked_to->cid,
        dst->process->pid, dst->cid,
        IFTYPE(header), METHODTYPE(header));

    // Copy payloads.
    receiver->buffer.header = header;
    receiver->buffer.sent_from = linked_to->cid;
    receiver->buffer.payloads[0] = a0;
    receiver->buffer.payloads[1] = a1;
    receiver->buffer.payloads[2] = a2;
    receiver->buffer.payloads[3] = a3;

    thread_resume(receiver);
    restore_irq(&irqstate);
    return ERROR_NONE;

slowpath:
    {
        header_t result = sys_send_slowpath(src, linked_to, header, a0, a1, a2, a3);
        restore_irq(&irqstate);
        return result;
    }
}


struct msg *sys_recv(channel_t ch) {
    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *src = get_channel_by_id(ch);
    if (unlikely(!src)) {
        WARN("sys_recv: @%d no such channel", ch);
        restore_irq(&irqstate);
        return ERROR_PTR(ERROR_INVALID_CH);
    }

    // Try to get the receiver right.
    struct thread *current_thread = CPUVAR->current;
    if (unlikely(!atomic_compare_and_swap(&src->receiver, NULL, current_thread))) {
        restore_irq(&irqstate);
        return ERROR_PTR(ERROR_CH_IN_USE);
    }

    if (unlikely(src->notifications)) {
receive_notification:
        current_thread->buffer.header = NOTIFICATION_MSG;
        current_thread->buffer.sent_from = src->cid;
        current_thread->buffer.payloads[0] = src->notifications;
        src->notifications = 0;
        src->receiver = NULL;
        restore_irq(&irqstate);
        return &current_thread->buffer;
    }

    // Wait for the sender.
    // Resume a thread in the wait queue.
    if (likely(src->sender)) {
        // The sender waits for us. Resume it and wait for it.
        thread_resume(src->sender);
    } else {
        struct waitqueue *wq = waitqueue_list_pop(&src->wq);
        if (wq) {
            thread_resume(wq->thread);
            kfree(wq);
        }
    }

    current_thread->receiving = src;
    thread_block_current();

    if (!src->sender) {
        // We have received a notification.
        goto receive_notification;
    }

    // Receiver sent a reply message and resumed the sender thread. Do recv
    // work. Release sender/receiver rights and return the receiver message.
    src->receiver = NULL;
    src->sender = NULL;
    current_thread->receiving = NULL;
    restore_irq(&irqstate);
    return &current_thread->buffer;
}


channel_t sys_connect(channel_t server) {
    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *ch = get_channel_by_id(server);
    if (!ch) {
        DEBUG("sys_connect: @%d no such channel", server);
        restore_irq(&irqstate);
        return ERROR_INVALID_CH;
    }

    struct channel *linked_to = ch->linked_to;
    if (!linked_to) {
        DEBUG("sys_connect: @%d not linked", ch);
        restore_irq(&irqstate);
        return ERROR_CH_NOT_LINKED;
    }

    restore_irq(&irqstate);
    return channel_connect(linked_to, CPUVAR->current->process);
}


channel_t sys_link(channel_t ch1, channel_t ch2) {
    /* TODO */
    return 0;
}


channel_t sys_transfer(channel_t ch, channel_t dest) {
    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *from = get_channel_by_id(ch);
    if (!from) {
        return ERROR_INVALID_CH;
    }

    struct channel *to = get_channel_by_id(dest);
    if (!to) {
        return ERROR_INVALID_CH;
    }

    transfer_to(from, to);
    restore_irq(&irqstate);
    return ERROR_NONE;
}


channel_t sys_discard(payload_t ool0, payload_t ool1, payload_t ool2, payload_t ool3) {
    /* TODO */
    return 0;
}

/* This function would be called in an interrupt context: don't
  use mutexes and printk nor they cause a dead lock! */
error_t do_notify(struct process *proc, channel_t ch, payload_t and_mask, payload_t or_mask) {
    irqstate_t irqstate;
    save_and_disable_irq(&irqstate);

    struct channel *src = get_channel_by_id_of(proc, ch);
    if (!src) {
        return ERROR_INVALID_CH;
    }

    struct channel *linked_to = src->linked_to;
    struct channel *dst = linked_to->transfer_to ?: linked_to;
    if (!dst) {
        return ERROR_INVALID_CH;
    }

    dst->notifications = (dst->notifications & and_mask) | or_mask;

    struct thread *receiver = dst->receiver;
    if (receiver) {
        thread_resume(receiver);
    }

    DEBUG("notify: %p @%d.%d", dst->notifications, dst->process->pid, dst->cid);
    restore_irq(&irqstate);
    return ERROR_NONE;
}

/* This function would be called in an interrupt context: don't
  use mutexes and printk nor they cause a dead lock! */
error_t sys_notify(channel_t ch, payload_t and_mask, payload_t or_mask) {
    return do_notify(CPUVAR->current->process, ch, and_mask, or_mask);
}


header_t invalid_syscall(void) {
    WARN("invalid syscall");
    return 0;
}
