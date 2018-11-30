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
namespace io_server {

void handle_allocate(
    channel_t __ch
    , resea::interfaces::io::space_t iospace
    , uintptr_t addr
    , size_t size
);
void handle_release(
    channel_t __ch
    , resea::interfaces::io::space_t iospace
    , uintptr_t addr
);

} // namespace io
} // namespace kernel
