#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include "handler.h"

namespace kernel {
namespace datetime_server {

/** handles datetime.set_oneshot_timer */
void handle_set_oneshot_timer(channel_t __ch, channel_t ch, uintmax_t msec) {

}

} // namespace datetime_server
} // namespace kernel
