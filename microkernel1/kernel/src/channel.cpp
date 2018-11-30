#include "kernel.h"
#include "thread.h"
#include "channel.h"
#include <resea.h>


namespace {
    struct kernel::channel::server servers[SERVERS_MAX];
    mutex_t servers_lock;
}

namespace kernel {
namespace channel {

// Allocates a channel ID in the thread group.
ident_t alloc_id(struct thread::thread_group *group) {
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


// Returns the pointer to the channel struct of the channel ID.
struct thread::channel *get_channel(struct thread::thread_group *group, channel_t ch) {

    return &group->channels[ch];
}


result_t transfer_to(struct thread::thread_group *group, channel_t src, channel_t dst) {

    group->channels[src].used        = true;
    group->channels[src].transfer_to = dst;

    DEBUG("transfer @%d:%d => @%d", group->id, src, dst);
    return OK;
}


result_t link(struct thread::thread_group *group1, channel_t ch1,
              struct thread::thread_group *group2, channel_t ch2) {

    group1->channels[ch1].used         = true;
    group1->channels[ch1].linked_group = group2->id;
    group1->channels[ch1].linked_to    = ch2;
    group2->channels[ch2].used         = true;
    group2->channels[ch2].linked_group = group1->id;
    group2->channels[ch2].linked_to    = ch1;

    DEBUG("connect @%d:%d <=> @%d:%d", group1->id, ch1, group2->id, ch2);
    return OK;
}


// Registers the channel as a server in `interface`.
result_t register_server(channel_t ch, interface_t interface) {

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
    servers[i].group_id  = thread::get_current_thread_group()->id;
    servers[i].ch        = ch;
    servers[i].interface = interface;

    INFO("registered server (%d)", interface >> 12);
    unlock_mutex(&servers_lock);
    return OK;
}


// Connects the channel `ch` to a server specified in `interface`.
result_t connect(channel_t ch, interface_t interface) {
    struct thread::thread_group *current_group, *server_group;
    bool retried = false;
    channel_t server_new_ch;

retry:
    lock_mutex(&servers_lock);

    // search `servers` for an appreciate server
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
            WARN("server (%d) not found, retrying", interface >> 12);
            retried = true;
        }

        goto retry;
    }

    INFO("found server (%d)", interface >> 12);
    current_group = thread::get_current_thread_group();
    server_group  = thread::get_thread_group(servers[i].group_id);

    server_new_ch = alloc_id(server_group);
    channel::transfer_to(server_group, server_new_ch, servers[i].ch);
    channel::link(current_group, ch, server_group, server_new_ch);

    INFO("connected (%d)", interface >> 12);
    return OK;
}


void init() {

    INFO("initializing the channel system");
    init_mutex(&servers_lock, MUTEX_UNLOCKED);

    for (int i=0; i < SERVERS_MAX; i++) {
        servers[i].used = false;
    }
}

} // namespace channel
} // namespace kernel
