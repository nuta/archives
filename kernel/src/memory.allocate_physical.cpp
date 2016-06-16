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
namespace memory_server {

/** handles memory.allocate_physical */
void handle_allocate_physical(channel_t __ch, paddr_t paddr, size_t size, uint32_t flags) {
     result_t r;
     uintptr_t addr;
     paddr_t r_paddr;

     r = kernel_allocate_memory_at(paddr, size, flags, &addr, &r_paddr);
     resea::interfaces::memory::send_allocate_physical_reply(__ch, r, addr, r_paddr);
}

} // namespace memory_server
} // namespace kernel
