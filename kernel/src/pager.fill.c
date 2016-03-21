#include <resea.h>
#include <resea/pager.h>


/** handles pager.fill */
void kernel_pager_fill(channel_t __ch, ident_t id, offset_t offset, size_t size) {

    send_pager_fill_reply(__ch, 
        OK, MOVE(allocMemory(size, ALLOCMEM_NORMAL)), size);
}
