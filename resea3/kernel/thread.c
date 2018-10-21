#include "printk.h"
#include "list.h"
#include "memory.h"
#include "thread.h"
#include "process.h"

static tid_t last_tid = 1;

tid_t allocate_tid(void) {
    // TODO: wrap
    return atomic_fetch_and_add(&last_tid, 1);
}

void free_tid(tid_t tid) {
    // TODO:
}


/*
 *  Wait until the all CPUs have escaped the ciritical section. Callee
 *  must *not* in a critical section.
 */
void sync_ciritical_section(void) {
#ifdef SMP
#error "not yet implemented"
    //  TODO:
    //
    //  Add `set_cpu_var(left_critical_section, 1)' in thread_switch() and wait for them:
    //
    //  for_each_cpu(cpu) {
    //      set_cpu_var(left_critical_section, 0);
    //  }
    //
    //  for_each_cpu(cpu) {
    //      while (read_cpu_var(left_critical_section) == 0) {
    //          thread_switch();
    //      }
    //  }
    //
#else
    // Do noting in UP systems.
#endif
}


struct thread *thread_create(struct process *process, uptr_t start, uptr_t arg) {
    bool is_kernel_thread = process == kernel_process;
    struct thread *thread = kmalloc(sizeof(*thread), KMALLOC_NORMAL);

    uptr_t stack;
    size_t stack_size;
    if (is_kernel_thread) {
        stack_size = 0x1000;
        stack = (uptr_t) kmalloc(stack_size, KMALLOC_NORMAL);
    } else {
        stack = process->next_stack_start;
        stack_size = 4 * PAGE_SIZE;

        process->next_stack_start = stack + stack_size;
        add_vmarea(&process->vms, HEAP_ADDR, 0, HEAP_ADDR_SIZE,
            PAGE_USER | PAGE_WRITABLE, zeroed_pager, NULL);
    }

    thread->process = process;
    thread->tid = allocate_tid();
    thread->state = THREAD_BLOCKED;
    thread->stack = stack;
    thread->resumed_count = 0;
    thread->rq.thread = thread;
    thread->wq.thread = thread;
    thread->sending = NULL;
    thread->receiving = NULL;

    arch_create_thread(
        process, &thread->arch, is_kernel_thread,
        start, arg, stack, stack_size
    );
    thread_list_append(&process->threads, thread);

    DEBUG("kernel: created a thread #%d process=%p, start=%p (%s)",
        thread->tid, thread->process, start, is_kernel_thread ? "kernel" : "user");
    return thread;
}


static void do_thread_destroy(struct thread *thread) {
    if (!atomic_compare_and_swap(&thread->state, THREAD_WAIT_DESTROY, THREAD_DESTROYED)) {
        // The destruction is (being) done by other CPU. I think it won't be
        // happen if runqueue is CPU-local.
        return;
    }

    // Remove from the runqueue.
    kmutex_state_t irq_state = kmutex_lock_irq_disabled(&CPUVAR->runqueue_lock);
    runqueue_list_remove(&CPUVAR->runqueue, &thread->rq);
    kmutex_unlock_restore_irq(&CPUVAR->runqueue_lock, irq_state);

    // Remove from the thread list of its process.
    irq_state = kmutex_lock_irq_disabled(&thread->process->lock);
    thread_list_remove(&thread->process->threads, thread);
    kmutex_unlock_restore_irq(&thread->process->lock, irq_state);

    // FIXME: Do we need to disable interrupts here?
    // Release the receiver right if the thread has.
    if (thread->receiving) {
        atomic_compare_and_swap(&thread->receiving->receiver, thread, NULL);
    }

    // Release the sender right and remove from the wait queue if exists.
    if (thread->sending) {
        atomic_compare_and_swap(&thread->sending->sender, thread, NULL);

        irq_state = kmutex_lock_irq_disabled(&thread->sending->lock);
        waitqueue_list_remove(&thread->sending->wq, &thread->wq);
        kmutex_unlock_restore_irq(&thread->sending->lock, irq_state);
    }

    // Wait until the all CPUs have escaped the ciritical section so that
    // there are no references to the thread.
    sync_ciritical_section();

    // Release allocated resources.
    free_tid(thread->tid);
    if (thread->process == kernel_process) {
        kfree((void *) thread->stack);
    }

    // Release remaining resources and finish the destruction.
    arch_destroy_thread(&thread->arch);
    kfree(thread);
}

static void append_to_runqueue(struct thread *thread) {
    kmutex_state_t state = kmutex_lock_irq_disabled(&CPUVAR->runqueue_lock);
    runqueue_list_append(&CPUVAR->runqueue, &thread->rq);
    kmutex_unlock_restore_irq(&CPUVAR->runqueue_lock, state);
}


void thread_destroy(struct thread *thread) {
    // TODO: Determine if `thread' is an idle thread by
    // testing flags in MP.
    if (thread == CPUVAR->idle_thread) {
        PANIC("idle thread can't be destroyied");
    }

    thread->state = THREAD_WAIT_DESTROY;
    append_to_runqueue(thread);
}

// `destroyed_thread' will never be run. Used for simplify arch_switch() implementation.
struct thread *destroyed_thread;

NORETURN void thread_destroy_current(void) {
    CPUVAR->current = destroyed_thread;
    thread_destroy(CPUVAR->current);
    thread_switch();
    PANIC("thread_switch() returned to thread_destroy_current()");
}


static void thread_switch_to(struct thread *prev, struct thread *next) {
    CPUVAR->current = next;

    if (next->process != kernel_process) {
        arch_switch_vmspace(&next->process->vms.arch);
    }

    arch_switch(&prev->arch, &next->arch);
}


void thread_resume(struct thread *thread) {
    int prev = atomic_fetch_and_add(&thread->resumed_count, 1);
    if (prev == 0) {
        thread->state = THREAD_RUNNABLE;
        append_to_runqueue(thread);
    }
}


void thread_block(struct thread *thread) {
    int prev = atomic_fetch_and_sub(&thread->resumed_count, 1);
    if (prev == 1) {
        thread->state = THREAD_BLOCKED;
    }
}

void thread_block_current(void) {
    struct thread *current = CPUVAR->current;
    atomic_fetch_and_sub(&current->resumed_count, 1);
    if (current->resumed_count <= 0) {
        current->state = THREAD_BLOCKED;
        thread_switch();
    }
}


void thread_switch(void) {
    // TODO: implement a fair and smart scheduler
    kmutex_state_t state;
    struct runqueue *next;
retry:
    state = kmutex_lock_irq_disabled(&CPUVAR->runqueue_lock);
    next = runqueue_list_pop(&CPUVAR->runqueue);
    kmutex_unlock_restore_irq(&CPUVAR->runqueue_lock, state);

    if (next) {
        if (CPUVAR->current->state == THREAD_RUNNABLE) {
            // Add the current thread to the runqueue.
            append_to_runqueue(CPUVAR->current);
        }

        if (next->thread->state == THREAD_WAIT_DESTROY) {
            do_thread_destroy(next->thread);
            goto retry;
        }

        thread_switch_to(CPUVAR->current, next->thread);
        return;
    }

    if (CPUVAR->current->state == THREAD_RUNNABLE) {
        // No other thread to run. Resume the current thread.
        return;
    }

    // No threads are runnable. Resume the idle thread.
    thread_switch_to(CPUVAR->current, CPUVAR->idle_thread);
}


void thread_init(void) {
     runqueue_list_init(&CPUVAR->runqueue);
     kmutex_init(&CPUVAR->runqueue_lock, KMUTEX_UNLOCKED);

    // Create an idle thread. We specify NULL as start address because it won't
    // be used.
    CPUVAR->idle_thread = thread_create(kernel_process, (uptr_t) NULL, 0);
    CPUVAR->current = CPUVAR->idle_thread;
    destroyed_thread = thread_create(kernel_process, (uptr_t) NULL, 0);
}
