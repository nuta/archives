#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <hal.h>

/*
 *  Threading
 */
#define THREAD_NUM_MAX 256
#define CHANNELS_MAX 64

struct channel {
    ident_t    dest;    // thread group
    channel_t  dest_ch;
    channel_t  transfer_to;
    bool       used;
    uintptr_t  handler;
    uintptr_t  buf;
    size_t     buf_num;
    uintmax_t  buf_index;
};

struct thread_group {
    ident_t    id;
    size_t  num; /* the number of threads belong to */
    mutex_t lock;
    struct vm_space vm;
    struct channel channels[CHANNELS_MAX];
};

struct thread {
    enum {
      THREAD_UNUSED,
      THREAD_RUNNABLE,
      THREAD_BLOCKED
    } status;
    struct hal_thread hal;
    struct thread_group *group;
};

void kernel_thread_startup(void);
struct thread_group *kernel_get_current_thread_group(void);
struct thread *kernel_get_current_thread(void);
ident_t kernel_create_thread_group(void);
struct thread_group *kernel_get_thread_group(ident_t group);
struct thread_group *kernel_get_thread_group_of(ident_t thread);
result_t  kernel_create_thread(ident_t group, const uchar_t *name, size_t name_size,
                           ident_t *r_thread, ident_t *r_group);
result_t kernel_set_thread(ident_t thread, uintptr_t entry, uintptr_t arg,
                       uintptr_t stack, size_t stack_size);
result_t kernel_set_thread_status(ident_t thread, int status);
void kernel_resume_next_thread(void);
void kernel_switch_thread(void);
void kernel_hard_switch_thread(void);

/*
 *  Memory
 */
void kernel_memory_startup(void);
void *kernel_allocate_memory(size_t size, uint32_t flags);
size_t kernel_get_dynamic_vpages_num(void);
uintptr_t kernel_vmalloc(size_t size);


/*
 *  Channel
 */
void kernel_channel_startup(void);
ident_t kernel_alloc_channel_id(struct thread_group *group);
result_t kernel_connect_channels(struct thread_group *group1, channel_t ch1,
                               struct thread_group *group2, channel_t ch2) ;
result_t kernel_connect_channel(channel_t ch, interface_t interface);
result_t kernel_register_channel(channel_t ch, interface_t interface);
result_t kernel_transfer_to(struct thread_group *group, channel_t src, channel_t dst);


/*
 *  Datetime
 */
void kernel_datetime_startup(void);
result_t kernel_get_date(uint32_t *year, uint32_t *date, uint32_t *nsec);


/*
 *  misc.
 */
handler_t kernel_handler;

#endif
