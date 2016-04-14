#include <resea.h>
#include "kernel.h"

#define SERVERS_MAX  256

struct server {
    bool       used;
    ident_t         group_id;
    channel_t    ch;
    interface_t  interface;
};

static struct server servers[SERVERS_MAX];
static mutex_t servers_lock;

ident_t kernel_alloc_channel_id(struct thread_group *group) {
    ident_t i;

    lock_mutex(&group->lock);

    for (i=1; i < CHANNELS_MAX; i++) {
        if (!group->channels[i].used)
            break;
    }

    if (group->channels[i].used) {
        unlock_mutex(&group->lock);
        WARN("too many channels");
        return 0;
    }

    group->channels[i].used = true;
    unlock_mutex(&group->lock);
    return i;
}


result_t kernel_transfer_to(struct thread_group *group, channel_t src, channel_t dst) {

    group->channels[src].used    = true;
    group->channels[src].transfer_to = dst;

    DEBUG("transfer @%d:%d => @%d", group->id, src, dst);
    return OK;
}


result_t kernel_link_channels(struct thread_group *group1, channel_t ch1,
                                 struct thread_group *group2, channel_t ch2) {

    group1->channels[ch1].used         = true;
    group1->channels[ch1].linked_group = group2->id;
    group1->channels[ch1].linked_to    = ch2;
    group2->channels[ch2].used         = true;
    group2->channels[ch2].linked_group = group1->id;
    group2->channels[ch2].linked_to    = ch1;

    DEBUG("connect @%d:%d <=> @%d:%d", group1->id, ch1, group2->id, ch2);
    return OK;
}


result_t kernel_register_channel(channel_t ch, interface_t interface) {

    lock_mutex(&servers_lock);

    int i;
    for (i=0; i < SERVERS_MAX; i++) {
        if (!servers[i].used) {
           break;
        }
    }

    if (i == SERVERS_MAX) {
        unlock_mutex(&servers_lock);
        INFO("too many servers");
        return E_NOSPACE;
    }

    servers[i].used      = true;
    servers[i].group_id  = kernel_get_current_thread_group()->id;
    servers[i].ch        = ch;
    servers[i].interface = interface;

    INFO("registered server (%d)", interface >> 24);
    unlock_mutex(&servers_lock);
    return OK;
}


result_t kernel_connect_channel(channel_t ch, interface_t interface) {
    struct thread_group *current_group, *server_group;
    bool retried = false;
    channel_t server_new_ch;

retry:
    lock_mutex(&servers_lock);

    int i;
    for (i=0; i < SERVERS_MAX; i++) {
        if (servers[i].used && servers[i].interface == interface) {
           break;
        }
    }

    unlock_mutex(&servers_lock);

    if (i == SERVERS_MAX) {
        // TODO: block current thread
        if (!retried) {
            WARN("server (%d) not found, retrying", interface >> 24);
            retried = true;
        }

        goto retry;
    }

    INFO("found server (%d)", interface >> 24);
    current_group = kernel_get_current_thread_group();
    server_group  = kernel_get_thread_group(servers[i].group_id);

    server_new_ch = kernel_alloc_channel_id(server_group);
    kernel_transfer_to(server_group, server_new_ch, servers[i].ch);
    kernel_link_channels(current_group, ch, server_group, server_new_ch);
    return OK;
}


void kernel_channel_startup(void) {

    INFO("initializing the channel system");
    init_mutex(&servers_lock, MUTEX_UNLOCKED);

    for (int i=0; i < SERVERS_MAX; i++) {
        servers[i].used = false;
    }
}
