#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/thread.h>
#include <resea/memory.h>
#include <resea/pager.h>
#include <resea/zeroed_pager.h>
#include <resea/channel.h>
#include <resea/datetime.h>
#include <resea/io.h>

namespace kernel {
void server_handler(channel_t __ch, payload_t *m);

namespace pager_server {
void handle_fill(
    channel_t __ch
    , ident_t id
    , offset_t offset
    , size_t size
);
} // namespace pager
namespace datetime_server {
void handle_get_date(
    channel_t __ch
);
} // namespace datetime
namespace datetime_server {
void handle_set_oneshot_timer(
    channel_t __ch
    , channel_t ch
    , uintmax_t msec
);
} // namespace datetime
namespace datetime_server {
void handle_set_interval_timer(
    channel_t __ch
    , channel_t ch
    , uintmax_t msec
);
} // namespace datetime
namespace datetime_server {
void handle_delay(
    channel_t __ch
    , uintmax_t msec
);
} // namespace datetime
namespace io_server {
void handle_allocate(
    channel_t __ch
    , resea::interfaces::io::space_t iospace
    , uintptr_t addr
    , size_t size
);
} // namespace io
namespace io_server {
void handle_release(
    channel_t __ch
    , resea::interfaces::io::space_t iospace
    , uintptr_t addr
);
} // namespace io
namespace thread_server {
void handle_create(
    channel_t __ch
    , ident_t group
    , uchar_t* name
    , size_t name_size
);
} // namespace thread
namespace thread_server {
void handle_delete(
    channel_t __ch
    , ident_t thread
);
} // namespace thread
namespace thread_server {
void handle_start(
    channel_t __ch
    , ident_t thread
);
} // namespace thread
namespace thread_server {
void handle_set(
    channel_t __ch
    , ident_t thread
    , uintptr_t entry
    , uintptr_t arg
    , uintptr_t stack
    , size_t stack_size
);
} // namespace thread
namespace thread_server {
void handle_get_current_thread(
    channel_t __ch
);
} // namespace thread
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
} // namespace memory
namespace memory_server {
void handle_unmap(
    channel_t __ch
    , uintptr_t addr
);
} // namespace memory
namespace memory_server {
void handle_get_page_size(
    channel_t __ch
);
} // namespace memory
namespace memory_server {
void handle_allocate(
    channel_t __ch
    , size_t size
    , uint32_t flags
);
} // namespace memory
namespace memory_server {
void handle_release(
    channel_t __ch
    , uintptr_t addr
);
} // namespace memory
namespace memory_server {
void handle_allocate_physical(
    channel_t __ch
    , paddr_t paddr
    , size_t size
    , uint32_t flags
);
} // namespace memory
namespace channel_server {
void handle_connect(
    channel_t __ch
    , uintmax_t channel
    , interface_t interface
);
} // namespace channel
namespace channel_server {
void handle_register(
    channel_t __ch
    , uintmax_t channel
    , interface_t interface
);
} // namespace channel

} // namespace kernel
