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
namespace pager_server {

void handle_fill(
    channel_t __ch
    , ident_t id
    , offset_t offset
    , size_t size
);

} // namespace pager
} // namespace kernel
