#include "types.h"
#include "printk.h"
#include "ipc.h"
#include <resea/exit.h>
#include <resea/logging.h>
#include <resea/discovery.h>
#include <resea/io.h>
#include "thread.h"
#include "process.h"
#include "ipc.h"
#include "server.h"

struct channel *kernel_channel;
struct service *services;
struct client *clients;
kmutex_t logging_lock;

static struct process *get_callee_process(channel_t from) {
    struct channel *ch = get_channel_by_id(from);
    return ch->linked_to->process;
}


static inline void handle_exit_exit(channel_t from, u32_t error) {
    struct process *caller = kernel_process->channels[from - 1].linked_to->process;
    DEBUG("exit.exit: pid=%d", caller->pid);
    /* TODO */
}


static inline error_t handle_logging_emit(channel_t from, string_t str, usize_t length) {
    kmutex_state_t mstate = kmutex_lock_irq_disabled(&logging_lock);

    for (usize_t i = 0; i < length; i++) {
        arch_putchar(str[i]);
    }

    kmutex_unlock_restore_irq(&logging_lock, mstate);
    return ERROR_NONE;
}


static inline error_t handle_discovery_register(channel_t from, u32_t service_type, channel_t server) {
    DEBUG("discovery.register: service=%d (%d)", service_type, server);

    struct service *service = kmalloc(sizeof(*service), KMALLOC_NORMAL);
    service->service_type = service_type;
    service->server = server;
    service_list_append(&services, service);

    /* Connect pending clients. */
    for (struct client *c = clients; c != NULL;) {
        if (c->service_type == service_type) {
            channel_t client = ipc_connect(service->server);
            ipc_send(c->ch, DISCOVERY_CONNECT_REPLY_HEADER, client, 0, 0, 0);
        }

        struct client *next = c->next;
        client_list_remove(&clients, c);
        c = next;
    }

    return ERROR_NONE;
}


static inline error_t handle_discovery_connect(channel_t from, u32_t service_type, channel_t *client) {
    DEBUG("discovery.connect: service=%d", service_type);

    for (struct service *service = services; service != NULL; service = service->next) {
        if (service->service_type == service_type) {
            *client = ipc_connect(service->server);
            return ERROR_NONE;
        }
    }

    struct client *c = kmalloc(sizeof(*client), KMALLOC_NORMAL);
    c->service_type = service_type;
    c->ch = from;
    client_list_append(&clients, c);
    return ERROR_DONT_REPLY;
}


static inline error_t handle_io_ioalloc(channel_t from, u32_t base, usize_t length) {
    // TODO: Allow new threads in its process to perform io operations after this
    //       ioalloc.

    struct process *proc = get_callee_process(from);
    for (struct thread *t = proc->threads; t != NULL; t = t->next) {
        arch_allow_io(&t->arch);
    }

    return ERROR_NONE;
}


static inline error_t handle_io_pmalloc(channel_t from, uptr_t vaddr, uptr_t paddr,
                                        usize_t length, uptr_t *vaddr_allocated,
                                        uptr_t *paddr_allocated) {
    if (vaddr != 0) {
        return ERROR_NOT_IMPLEMENTED;
    }

    struct process *proc = get_callee_process(from);
    struct vmspace *vms = &proc->vms;
    int flags = PAGE_WRITABLE | (IS_KERNEL_PROCESS(proc) ? 0 : PAGE_USER);
    length = ROUND_UP(length, PAGE_SIZE);
    vaddr = valloc(vms, length);

    if (paddr == 0) {
        paddr = alloc_pages(length, KMALLOC_NORMAL);
    }

    arch_link_page(&vms->arch, vaddr, paddr, length / PAGE_SIZE, flags);

    *vaddr_allocated = vaddr;
    *paddr_allocated = paddr;
    return ERROR_NONE;
}


void kernel_server_mainloop(channel_t server) {
    channel_t from;
    payload_t a0, a1, a2, a3;
    payload_t r0 = 0, r1 = 0, r2 = 0, r3 = 0;
    header_t header = ipc_recv(server, &from, &a0, &a1, &a2, &a3);
    for (;;) {
        error_t error = 0;
        switch (MSGTYPE(header)) {
            case EXIT_EXIT_MSG:
                handle_exit_exit(from, (error_t) a0);
                /* The caller thread is terminated. Needless to reply. */
                error = ERROR_DONT_REPLY;
                break;
            case LOGGING_EMIT_MSG:
                error = handle_logging_emit(from, (string_t) a0, (usize_t) a1);
                header = LOGGING_EMIT_REPLY_HEADER | (error << ERROR_OFFSET);
                break;
            case DISCOVERY_REGISTER_MSG:
                error = handle_discovery_register(from, (u32_t) a0, (channel_t) a1);
                header = DISCOVERY_REGISTER_REPLY_HEADER | (error << ERROR_OFFSET);
                break;
            case DISCOVERY_CONNECT_MSG:
                error = handle_discovery_connect(from, (u32_t) a0, (channel_t *) &r0);
                header = DISCOVERY_CONNECT_REPLY_HEADER | (error << ERROR_OFFSET);
                break;
            case IO_IOALLOC_MSG:
                error = handle_io_ioalloc(from, (u32_t) a0, (usize_t) a1);
                header = IO_IOALLOC_REPLY_HEADER | (error << ERROR_OFFSET);
                break;
            case IO_PMALLOC_MSG:
                error = handle_io_pmalloc(from, (uptr_t) a0, (uptr_t) a1, (usize_t) a2, (uptr_t *) &r0, (uptr_t *) &r1);
                header = IO_PMALLOC_REPLY_HEADER | (error << ERROR_OFFSET);
                break;
            default:
                /* Unknown message. */
                DEBUG("kernel: unknown message %d.%d", MSG_SERVICE_ID(header), MSG_ID(header));
                header = ERROR_UNKNOWN_MSG << ERROR_OFFSET;
                break;
        }

        if (error == ERROR_DONT_REPLY) {
            header = ipc_recv(server, &from, &a0, &a1, &a2, &a3);
        } else {
            header = ipc_replyrecv(&from, header, r0, r1, r2, r3, &a0, &a1, &a2, &a3);
        }
    }
}


void kernel_server(void) {
    kernel_server_mainloop(kernel_channel->cid);
}


void kernel_server_init(void) {
    service_list_init(&services);
    client_list_init(&clients);
    kmutex_init(&logging_lock, KMUTEX_UNLOCKED);

    kernel_channel = channel_create(kernel_process);
    if (kernel_channel == NULL) {
        PANIC("failed to create a kernel channel");
    }

    thread_resume(thread_create(kernel_process, (uptr_t) kernel_server, 0));
}
