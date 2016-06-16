#include "kernel.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/pager.h>
#include "kernel.h"


namespace kernel {
namespace pager_server {

/** handles pager.fill */
void handle_fill(channel_t __ch, ident_t id, offset_t offset, size_t size) {

    resea::interfaces::pager::sendas_fill_reply(__ch,
        OK, PAYLOAD_INLINE,
        kernel_allocate_memory(size, resea::interfaces::memory::ALLOC_NORMAL), PAYLOAD_MOVE_OOL,
        size, PAYLOAD_INLINE);
}

} // namespace pager_server
} // namespace kernel
