#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>

/** handles memory.unmap */
void kernel_memory_unmap(channel_t __ch, uintptr_t addr) {

    WARN("unimplemented");
    send_memory_map_reply(__ch, OK);
}
