#include "kernel.h"
#include "channel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>


namespace kernel {
namespace channel_server {


void handle_connect(channel_t __ch, uintmax_t channel, interface_t interface) {

    channel::connect(channel, interface);
    resea::interfaces::channel::send_connect_reply(__ch, OK);
}


void handle_register(channel_t __ch, uintmax_t channel, interface_t interface) {

    channel::register_server(channel, interface);
    resea::interfaces::channel::send_register_reply(__ch, OK);
}


} // namespace channel_server
} // namespace kernel
