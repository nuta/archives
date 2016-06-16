#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>

namespace kernel {
namespace memory_server {

/** handles memory.unmap */
void handle_unmap(channel_t __ch, uintptr_t addr) {

    WARN("unimplemented");
    resea::interfaces::memory::send_map_reply(__ch, OK);
}

} // namespace memory_server
} // namespace kernel
