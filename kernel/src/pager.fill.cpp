#include "kernel.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/pager.h>
#include "kernel.h"


/** handles pager.fill */
void kernel_pager_fill(channel_t __ch, ident_t id, offset_t offset, size_t size) {

    sendas_pager_fill_reply(__ch,
        OK, PAYLOAD_INLINE,
        kernel_allocate_memory(size, MEMORY_ALLOC_NORMAL), PAYLOAD_MOVE_OOL,
        size, PAYLOAD_INLINE);
}
