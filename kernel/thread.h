#ifndef __THREAD_H__
#define __THREAD_H__

#include <resea/types.h>

typedef uint32_t tid_t;

#define THREAD_RUNNABLE 1
#define THREAD_BLOCKED 2

struct thread {
    uint32_t flags;
    tid_t tid;
    struct arch_regs regs;
};

static inline void thread_set_state(struct thread *thread, int state) {
    thread->flags = (thread->flags & ~3) | state;
}

struct process;

tid_t allocate_tid(void);
struct thread *thread_create(struct process *process, uintptr_t start, uintptr_t arg);
void thread_switch_to(struct thread *thread);
void thread_switch(void);
void thread_init(void);

#endif
