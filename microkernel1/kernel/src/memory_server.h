#include "kernel.h"
#include <resea.h>
#include <resea/memory.h>
#include <resea/zeroed_pager.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/kernel.h>
#include <resea/pager.h>
#include <resea/channel.h>
#include <resea/thread.h>

namespace kernel {
namespace memory_server {

void handle_map(
    channel_t __ch
    , ident_t group
    , uintptr_t addr
    , size_t size
    , channel_t pager
    , ident_t pager_arg
    , offset_t offset
);
void handle_unmap(
    channel_t __ch
    , uintptr_t addr
);
void handle_get_page_size(
    channel_t __ch
);
void handle_allocate(
    channel_t __ch
    , size_t size
    , uint32_t flags
);
void handle_release(
    channel_t __ch
    , uintptr_t addr
);
void handle_allocate_physical(
    channel_t __ch
    , paddr_t paddr
    , size_t size
    , uint32_t flags
);

} // namespace memory
} // namespace kernel
