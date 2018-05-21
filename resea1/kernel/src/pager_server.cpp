#include "kernel.h"
#include "memory.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/pager.h>


namespace kernel {
namespace pager_server {

/** handles pager.fill */
void handle_fill(channel_t __ch, ident_t id, offset_t offset, size_t size) {
    void *p;

    p = memory::allocate(size, resea::interfaces::memory::ALLOC_NORMAL);

    resea::interfaces::pager::sendas_fill_reply(__ch,
        OK,   PAYLOAD_INLINE,
        p,    PAYLOAD_MOVE_OOL,
        size, PAYLOAD_INLINE);
}

} // namespace pager_server
} // namespace kernel
