#include <mutex.h>
#include <arch.h>
#include <logging.h>
#include "resources.h"
#include "thread.h"
#include "process.h"
#include "kmalloc.h"
#include "panic.h"


static tid_t last_tid = 0;
static mutex_t allocate_tid_lock = MUTEX_INITIALIZER;

tid_t allocate_tid(void) {
    tid_t tid;

    mutex_lock(&allocate_tid_lock);
    last_tid++;
    tid = last_tid;
    mutex_unlock(&allocate_tid_lock);

    return tid;
}


struct thread *get_current_thread(void) {

    // TODO: The cost of this function is O(n). We need a more and more
    //       faster algorithm.

    tid_t tid = arch_get_current_tid();

    struct process *proc = resources->processes;

    while (proc) {
        struct thread *t = proc->threads;
        while (t != NULL) {
            if (t->tid == tid) {
                return t;
            }

            t = t->next;
        }
        proc = proc->next;
    }

    PANIC("failed to get the current thread (TID: %d)", tid);
    return NULL;
}


struct thread *create_thread(struct process *process, uintptr_t start,
                             uintmax_t arg) {
    struct thread *thread;
    uintptr_t stack;
    const size_t stack_size = DEFAULT_THREAD_STACK_SIZE;

    // allocate memory for thread and stack
    thread = (struct thread *) kmalloc(sizeof(struct thread), KMALLOC_NORMAL);
    if (!thread) {
        WARN("failed to allocate a thread");
        return NULL;
    }

    stack = (uintptr_t) kmalloc(stack_size, KMALLOC_NORMAL);
    if (!stack) {
        WARN("failed to allocate a stack");
        kfree(thread);
        return NULL;
    }

    // append `thread` into the thread list in the process
    mutex_lock(&process->threads_lock);
    thread->next = NULL;
    if (process->threads) {
        struct thread *t = process->threads;
        struct thread *last = t;
        while (t != NULL) {
            last = t;
            t    = t->next;
        }
        last->next = thread;
    } else {
        process->threads = thread;
    }

    mutex_unlock(&process->threads_lock);

    // set other variables
    thread->tid     = allocate_tid();
    thread->process = process;
    arch_create_thread(&thread->arch, start, arg, stack, stack_size);

    return thread;
}


void destroy_thread(struct thread *thread) {
}


void set_thread_state(struct thread *thread, int state) {

    thread->state = state;
}


void resume_thread(struct thread *thread) {

    set_thread_state(thread, THREAD_RUNNABLE);
    // TODO add to runqueue if thread is not in it
}


void start_thread(struct thread *thread) {

    set_thread_state(thread, THREAD_RUNNABLE);
    queue_put(&resources->run_queue, thread);
}


void yield(void) {
    if (arch_yield(&get_current_thread()->arch)) {
        return;
    } else {
        struct thread *next;
        queue_put(&resources->run_queue, get_current_thread());
        next = (struct thread *) queue_get(&resources->run_queue);
        arch_switch_thread(next->tid, &next->arch);
    }
}
