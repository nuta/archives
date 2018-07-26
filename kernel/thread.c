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
    thread->resumed_count = 0;
    thread->rq.thread = thread;

    arch_create_thread(&thread->arch, is_kernel_thread,
        start, arg, stack, stack_size);
    thread_list_append(&process->threads, thread);

    DEBUG("kernel: created a thread #%d process=%p, start=%p (%s)",
        thread->tid, thread->process, start, is_kernel_thread ? "kernel" : "user");
    return thread;
}


void thread_destroy(struct thread *thread) {
    if (thread == CPUVAR->idle_thread) {
        PANIC("idle thread can't be destroyied");
    }

    kmutex_state_t irq_state = kmutex_lock_irq_disabled(&CPUVAR->runqueue_lock);
    for (struct runqueue *rq = CPUVAR->runqueue; rq != NULL; rq = rq->next) {
        if (rq->thread == thread) {
            runqueue_list_remove(&CPUVAR->runqueue, rq);
            kfree(rq);
            break;
        }
    }
    kmutex_unlock_restore_irq(&CPUVAR->runqueue_lock, irq_state);

    irq_state = kmutex_lock_irq_disabled(&thread->process->lock);
    thread_list_remove(&thread->process->threads, thread);
    kmutex_unlock_restore_irq(&thread->process->lock, irq_state);

    arch_destroy_thread(&thread->arch);
    kfree(thread);
}


NORETURN void thread_destroy_current(void) {
    struct thread *thread = CPUVAR->current;
    if (thread == CPUVAR->idle_thread) {
        PANIC("idle thread can't be destroyied");
    }

    PANIC("%s: not yet implemented", __func__);
}


void thread_switch_to(struct thread *next) {
    struct thread *current = CPUVAR->current;
    CPUVAR->current = next;

    if (next->process != kernel_process) {
        arch_switch_vmspace(&next->process->vms.arch);
    }

    arch_switch(&current->arch, &next->arch);
}


void thread_resume(struct thread *thread) {
    int prev = atomic_fetch_and_add(&thread->resumed_count, 1);
    if (prev == 0) {
        thread->state = THREAD_RUNNABLE;
        kmutex_state_t state = kmutex_lock_irq_disabled(&CPUVAR->runqueue_lock);
        runqueue_list_append(&CPUVAR->runqueue, &thread->rq);
        kmutex_unlock_restore_irq(&CPUVAR->runqueue_lock, state);
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

    kmutex_state_t state = kmutex_lock_irq_disabled(&CPUVAR->runqueue_lock);
    struct runqueue *next = runqueue_list_pop(&CPUVAR->runqueue);
    kmutex_unlock_restore_irq(&CPUVAR->runqueue_lock, state);

    if (next) {
        if (CPUVAR->current->state == THREAD_RUNNABLE) {
            // Add the current thread to the runqueue.
            thread_block(CPUVAR->current);
            thread_resume(CPUVAR->current);
        }

        thread_switch_to(next->thread);
        return;
    }

    if (CPUVAR->current->state == THREAD_RUNNABLE) {
        // No other thread to run. Resume the current thread.
        return;
    }

    // No threads are runnable. Resume the idle thread. Assuming `runqueue` points
    // to the idle thread.
    struct thread *current_thread = CPUVAR->current;
    CPUVAR->current = CPUVAR->idle_thread;
    arch_switch(&current_thread->arch, &CPUVAR->idle_thread->arch);
}


void thread_init(void) {
     runqueue_list_init(&CPUVAR->runqueue);
     kmutex_init(&CPUVAR->runqueue_lock, KMUTEX_UNLOCKED);

    // Create an idle thread. We specify NULL as start address because it won't
    // be used.
    CPUVAR->idle_thread = thread_create(kernel_process, (uptr_t) NULL, 0);
    CPUVAR->current = CPUVAR->idle_thread;
}
