#ifndef __THREAD_H__
#define __THREAD_H__

#include <kernel/types.h>
#include "ipc.h"
#include "list.h"

typedef u32_t tid_t;

// The interval must be long enough. A short interval leads to
// recursive timer interrupt handling, kernel stack starvation,
// and eventually a nasty bug.
#define THREAD_SWITCH_INTERVAL ((50 * 1000) / TICK_HZ)

#define THREAD_RUNNABLE 1
#define THREAD_BLOCKED 2
#define THREAD_WAIT_DESTROY 3
#define THREAD_DESTROYED 4
#define THREAD_CHANNEL_CLOSED 5
#define KERNEL_STACK_SIZE 8192

struct runqueue {
    struct runqueue *next;
    struct thread *thread;
};

struct process;
struct thread {
    struct thread *next;
    struct process *process;
    int state;
    tid_t tid;
    uptr_t stack;
    struct arch_thread arch;
    int resumed_count; /* runnable if resumed_count > 0 */
    struct runqueue rq;
    struct waitqueue wq;
    struct msg buffer;
    struct channel *sending;   // The channel for which the blocked thread is waiting.
    struct channel *receiving; // The channel for which the blocked thread is waiting.
};

DEFINE_LIST(thread, struct thread)
DEFINE_LIST(runqueue, struct runqueue)

struct process;

tid_t allocate_tid(void);
void free_tid(tid_t tid);
void sync_ciritical_section();
struct thread *thread_create(struct process *process, uptr_t start, uptr_t arg);
void thread_destroy(struct thread *thread);
NORETURN void thread_destroy_current(void);
void thread_switch(void);
void thread_init(void);
void thread_resume(struct thread *thread);
void thread_block(struct thread *thread);
void thread_block_current(void);

#endif
