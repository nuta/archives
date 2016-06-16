#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include "kernel.h"


namespace kernel {
namespace channel_server {

/** handles channel.connect */
void handle_connect(channel_t __ch, uintmax_t channel, interface_t interface) {

    kernel_connect_channel(channel, interface);
    send_channel_connect_reply(__ch, OK);
}

} // namespace channel_server
} // namespace kernel
