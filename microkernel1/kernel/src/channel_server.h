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
namespace channel_server {

void handle_connect(
    channel_t __ch
    , uintmax_t channel
    , interface_t interface
);
void handle_register(
    channel_t __ch
    , uintmax_t channel
    , interface_t interface
);

} // namespace channel
} // namespace kernel
