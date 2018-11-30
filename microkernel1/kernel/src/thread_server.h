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
namespace thread_server {

void handle_create(
    channel_t __ch
    , ident_t group
    , uchar_t* name
    , size_t name_size
);
void handle_delete(
    channel_t __ch
    , ident_t thread
);
void handle_start(
    channel_t __ch
    , ident_t thread
);
void handle_set(
    channel_t __ch
    , ident_t thread
    , uintptr_t entry
    , uintptr_t arg
    , uintptr_t stack
    , size_t stack_size
);
void handle_get_current_thread(
    channel_t __ch
);

} // namespace thread
} // namespace kernel
