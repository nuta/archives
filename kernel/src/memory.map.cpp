#include "kernel.h"
#include <resea.h>
#include <string.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <hal.h>
#include "kernel.h"


/** handles memory.map */
void kernel_memory_map(channel_t __ch,
       ident_t group,
       uintptr_t addr,
       size_t size,
       channel_t pager,
       ident_t pager_arg,
       offset_t offset) {

    uint8_t flags;
    intmax_t i;
    struct thread_group *current = kernel_get_thread_group(group);
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
        send_memory_map_reply(__ch, OK);
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

   send_memory_map_reply(__ch, OK);
}
