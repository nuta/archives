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


/** handles channel.register */
void kernel_channel_register(channel_t __ch, uintmax_t channel, interface_t interface) {

    kernel_register_channel(channel, interface);
    send_channel_register_reply(__ch, OK);
}
