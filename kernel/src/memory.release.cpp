#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>

namespace kernel {
namespace memory_server {

/** handles memory.release */
void handle_release(channel_t __ch, uintptr_t addr) {

}

} // namespace memory_server
} // namespace kernel
