#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include "kernel.h"


/** handles memory.allocate */
void kernel_memory_allocate(channel_t __ch, size_t size, uint32_t flags) {
     result_t r;
     uintptr_t addr;
     paddr_t paddr;

     r = kernel_allocate_memory_at(0, size, flags, &addr, &paddr);
     send_memory_allocate_reply(__ch, r, addr);
}
