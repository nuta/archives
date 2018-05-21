#ifndef __KERNEL_CHANNEL_H__
#define __KERNEL_CHANNEL_H__

#include <resea.h>
#include "thread.h"

namespace kernel {
namespace channel {

#define SERVERS_MAX  256

struct server {
    bool         used;
    ident_t      group_id;
    channel_t    ch;
    interface_t  interface;
};

ident_t alloc_id(struct thread::thread_group *group);
result_t connect(channel_t ch, interface_t interface);
result_t register_server(channel_t ch, interface_t interface);
result_t link(struct thread::thread_group *group1, channel_t ch1,
              struct thread::thread_group *group2, channel_t ch2);
result_t transfer_to(struct thread::thread_group *group, channel_t src, channel_t dst);
struct thread::channel *get_channel(struct thread::thread_group *group, channel_t ch);
void init();

} // namespace channel
} // namespace kernel

#endif
