#ifndef __KERNEL_THREAD_H__
#define __KERNEL_THREAD_H__

#include <resea.h>
#include <hal.h>

namespace kernel { // namespace kernel
namespace thread { // namespace thread

#define CHANNELS_MAX 64
struct channel {
    ident_t    linked_group;
    channel_t  linked_to;
    channel_t  transfer_to;
    bool       used;
    uintptr_t  handler;
    uintptr_t  buf;
    size_t     buf_num;
    uintmax_t  buf_index;
};

#define THREAD_NUM_MAX 256
struct thread_group {
    ident_t  id;
    size_t   num; // the number of threads belong to
    bool     used;
    mutex_t  lock;
    struct   vm_space vm;
    struct   channel channels[CHANNELS_MAX];
};

enum thread_status {
  THREAD_UNUSED,
  THREAD_RUNNABLE,
  THREAD_BLOCKED
};

struct thread {
    enum thread_status status;
    struct hal_thread hal;
    struct thread_group *group;
};

result_t set_status(ident_t thread, enum thread_status status);
struct thread *get_current_thread(void);
struct thread_group *get_current_thread_group(void);
result_t create_thread(ident_t group, const uchar_t *name, size_t name_size,
                       ident_t *r_thread, ident_t *r_group);
result_t set_reg_state(ident_t thread, uintptr_t entry, uintptr_t arg,
                       uintptr_t stack, size_t stack_size);
struct thread_group *get_thread_group(ident_t group);
struct thread_group *get_thread_group_of(ident_t thread);
void hard_switch(void);
void init();
void yield();

} // namespace thread
} // namespace kernel

#endif
