#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>

namespace kernel {
namespace datetime_server {

/** handles datetime.delay */
void handle_delay(channel_t __ch, uintmax_t msec) {

    resea::interfaces::datetime::send_delay_reply(__ch, OK);
}

} // namespace datetime_server
} // namespace kernel
