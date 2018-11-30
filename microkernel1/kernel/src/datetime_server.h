#include "kernel.h"
#include <resea.h>
#include <resea/memory.h>
#include <resea/zeroed_pager.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/kernel.h>
#include <resea/pager.h>
#include <resea/channel.h>
#include <resea/thread.h>

namespace kernel {
namespace datetime_server {

void handle_get_date(
    channel_t __ch
);
void handle_set_oneshot_timer(
    channel_t __ch
    , channel_t ch
    , uintmax_t msec
);
void handle_set_interval_timer(
    channel_t __ch
    , channel_t ch
    , uintmax_t msec
);
void handle_delay(
    channel_t __ch
    , uintmax_t msec
);

} // namespace datetime
} // namespace kernel
