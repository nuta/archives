#include "kernel.h"
#include "memory.h"
#include "thread.h"
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

/** handles memory.allocate */
void handle_allocate(channel_t __ch, size_t size, uint32_t flags) {
     result_t r;
     uintptr_t addr;
     paddr_t paddr;

     r = memory::allocate_at(0, size, flags, &addr, &paddr);
     resea::interfaces::memory::send_allocate_reply(__ch, r, addr);
}


/** handles memory.allocate_physical */
void handle_allocate_physical(channel_t __ch, paddr_t paddr, size_t size, uint32_t flags) {
     result_t r;
     uintptr_t addr;
     paddr_t r_paddr;

     r = memory::allocate_at(paddr, size, flags, &addr, &r_paddr);
     resea::interfaces::memory::send_allocate_physical_reply(__ch, r, addr, r_paddr);
}


/** handles memory.get_page_size */
void handle_get_page_size(channel_t __ch) {

    resea::interfaces::memory::send_get_page_size_reply(__ch, PAGE_SIZE);
}


/** handles memory.map */
void handle_map(channel_t __ch,
       ident_t group,
       uintptr_t addr,
       size_t size,
       channel_t pager,
       ident_t pager_arg,
       offset_t offset) {

    uint8_t flags;
    intmax_t i;
    struct thread::thread_group *current = thread::get_thread_group(group);
    struct vm_space *vm_space = &current->vm;
    struct vm_area *area;

    flags = size & 0xff;
    size  = size & (~0xff);

    /* look for unused vm_area struct */
    lock_mutex(&current->lock);
    for (i=0; i < VM_AREA_MAX; i++) {
        area = &vm_space->areas[i];
        if (area->size == 0)
            break;
    }

    if (area->size > 0) {
        unlock_mutex(&current->lock);
        resea::interfaces::memory::send_map_reply(__ch, OK);
        return;
    }

    INFO("map: group=%d, num=%d, range=%p-%p, flags=%#2x",
         group, i, addr, addr + size - 1, flags);
    area->addr      = addr;
    area->size      = size;
    area->pager_arg = pager_arg;
    area->offset    = offset;
    area->flags     = flags;
    area->pager     = pager;
    vm_space->areas_num++;

    unlock_mutex(&current->lock);

   resea::interfaces::memory::send_map_reply(__ch, OK);
}


/** handles memory.release */
void handle_release(channel_t __ch, uintptr_t addr) {

    WARN("unimplemented");
}


/** handles memory.unmap */
void handle_unmap(channel_t __ch, uintptr_t addr) {

    WARN("unimplemented");
    resea::interfaces::memory::send_map_reply(__ch, OK);
}

} // namespace memory_server
} // namespace kernel
