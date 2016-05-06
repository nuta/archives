#include <resea.h>

void kernel_pager_fill(channel_t __ch, ident_t a0, offset_t a1, size_t a2);
void kernel_datetime_get_date(channel_t __ch);
void kernel_datetime_set_oneshot_timer(channel_t __ch, channel_t a0, uintmax_t a1);
void kernel_datetime_set_interval_timer(channel_t __ch, channel_t a0, uintmax_t a1);
void kernel_datetime_delay(channel_t __ch, uintmax_t a0);
void kernel_io_allocate(channel_t __ch, io_space_t a0, uintptr_t a1, size_t a2);
void kernel_io_release(channel_t __ch, io_space_t a0, uintptr_t a1);
void kernel_thread_create(channel_t __ch, ident_t a0, uchar_t* a1, size_t a2);
void kernel_thread_delete(channel_t __ch, ident_t a0);
void kernel_thread_start(channel_t __ch, ident_t a0);
void kernel_thread_block(channel_t __ch, ident_t a0);
void kernel_thread_unblock(channel_t __ch, ident_t a0);
void kernel_thread_set(channel_t __ch, ident_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3, size_t a4);
void kernel_thread_get_current_thread(channel_t __ch);
void kernel_memory_map(channel_t __ch, ident_t a0, uintptr_t a1, size_t a2, channel_t a3, ident_t a4, offset_t a5);
void kernel_memory_unmap(channel_t __ch, uintptr_t a0);
void kernel_memory_get_page_size(channel_t __ch);
void kernel_memory_allocate(channel_t __ch, size_t a0, uint32_t a1);
void kernel_memory_release(channel_t __ch, uintptr_t a0);
void kernel_memory_allocate_physical(channel_t __ch, paddr_t a0, size_t a1, uint32_t a2);
void kernel_channel_connect(channel_t __ch, uintmax_t a0, interface_t a1);
void kernel_channel_register(channel_t __ch, uintmax_t a0, interface_t a1);
