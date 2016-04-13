#include <resea.h>
#include <resea/pager.h>
#include "kernel.h"


/** handles pager.fill */
void kernel_pager_fill(channel_t __ch, ident_t id, offset_t offset, size_t size) {

    send_pager_fill_reply(__ch, 
        OK, MOVE(kernel_allocate_memory(size, ALLOCMEM_NORMAL)), size);
}
