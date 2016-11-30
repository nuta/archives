#include <mutex.h>
#include <arch.h>
#include <logging.h>
#include "resources.h"
#include "thread.h"
#include "process.h"
#include "kmalloc.h"
#include "panic.h"
#include "timer.h"


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

    // set other variables
    thread->process = process;
    thread->tid     = allocate_tid();
    thread->state   = THREAD_BLOCKED;
    thread->next    = NULL;
    arch_create_thread(&thread->arch, start, arg, stack, stack_size);

    // append `thread` into the thread list in the process
    mutex_lock(&process->threads_lock);

    if (process->threads) {
        struct thread *last = process->threads;
        while (last->next) {
            last = last->next;
        }

        last->next = thread;
    } else {
        process->threads = thread;
    }

    mutex_unlock(&process->threads_lock);

    DEBUG("created a new thread #%d", thread->tid);
    return thread;
}


void destroy_thread(struct thread *thread) {

    // TODO
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

    // Add the thread into runqueue.
    mutex_lock(&resources->runqueue_lock);

    int i;
    struct thread **runqueue = (struct thread **) &resources->runqueue;
    for (i = 0; i < resources->runqueue_num; i++) {
        if (runqueue[i] == NULL) {
            runqueue[i] = thread;
            mutex_unlock(&resources->runqueue_lock);
            return;
        }
    }

    // TODO
    WARN("runqueue is full");
    mutex_unlock(&resources->runqueue_lock);
}


void yield(void) {
    if (arch_yield(&get_current_thread()->arch)) {
        return;
    } else {
        // Look for the next thread to run.
        mutex_lock(&resources->runqueue_lock);
        int i;
        struct thread **runqueue = (struct thread **) &resources->runqueue;
        for (i = 0; runqueue[i]; i++) {
            if (runqueue[i]->state == THREAD_RUNNABLE) {
                mutex_unlock(&resources->runqueue_lock);
                arch_switch_thread(runqueue[i]->tid,
                                   &runqueue[i]->arch);
            }
        }

        // No threads to run.
        mutex_unlock(&resources->runqueue_lock);
        arch_halt_until(get_next_timeout());
    }
}
