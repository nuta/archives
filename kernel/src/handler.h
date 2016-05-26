#include "kernel.h"
#include <resea.h>

void kernel_pager_fill(
    channel_t __ch
    , ident_t id
    , offset_t offset
    , size_t size
);
void kernel_datetime_get_date(
    channel_t __ch
);
void kernel_datetime_set_oneshot_timer(
    channel_t __ch
    , channel_t ch
    , uintmax_t msec
);
void kernel_datetime_set_interval_timer(
    channel_t __ch
    , channel_t ch
    , uintmax_t msec
);
void kernel_datetime_delay(
    channel_t __ch
    , uintmax_t msec
);
void kernel_io_allocate(
    channel_t __ch
    , io_space_t iospace
    , uintptr_t addr
    , size_t size
);
void kernel_io_release(
    channel_t __ch
    , io_space_t iospace
    , uintptr_t addr
);
void kernel_thread_create(
    channel_t __ch
    , ident_t group
    , uchar_t* name
    , size_t name_size
);
void kernel_thread_delete(
    channel_t __ch
    , ident_t thread
);
void kernel_thread_start(
    channel_t __ch
    , ident_t thread
);
void kernel_thread_set(
    channel_t __ch
    , ident_t thread
    , uintptr_t entry
    , uintptr_t arg
    , uintptr_t stack
    , size_t stack_size
);
void kernel_thread_get_current_thread(
    channel_t __ch
);
void kernel_memory_map(
    channel_t __ch
    , ident_t group
    , uintptr_t addr
    , size_t size
    , channel_t pager
    , ident_t pager_arg
    , offset_t offset
);
void kernel_memory_unmap(
    channel_t __ch
    , uintptr_t addr
);
void kernel_memory_get_page_size(
    channel_t __ch
);
void kernel_memory_allocate(
    channel_t __ch
    , size_t size
    , uint32_t flags
);
void kernel_memory_release(
    channel_t __ch
    , uintptr_t addr
);
void kernel_memory_allocate_physical(
    channel_t __ch
    , paddr_t paddr
    , size_t size
    , uint32_t flags
);
void kernel_channel_connect(
    channel_t __ch
    , uintmax_t channel
    , interface_t interface
);
void kernel_channel_register(
    channel_t __ch
    , uintmax_t channel
    , interface_t interface
);
