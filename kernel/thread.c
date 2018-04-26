#include "memory.h"
#include "thread.h"
#include "process.h"

static tid_t last_tid = 0;

tid_t allocate_tid(void) {
    // FIXME
    return last_tid++;
}


struct thread *thread_create(struct process *process, uintptr_t start, uintptr_t arg) {
    struct thread *thread = kmalloc(sizeof(*thread), KMALLOC_NORMAL);

    thread->tid = allocate_tid();
    thread->flags = THREAD_BLOCKED;
    arch_init_regs(&thread->regs);
//    list_append(&process->threads, thread);
    return thread;
}


void thread_destroy(struct thread *thread) {

}


void thread_switch_to(struct thread *thread) {
    arch_switch_to(&thread->regs);
}


void thread_switch(void) {
}


void thread_init(void) {
}
