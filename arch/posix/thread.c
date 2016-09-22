#include "stdlibs.h"
#include <arch.h>
#include <logging.h>
#include <kernel/panic.h>


static tid_t current_tid; // TODO: make it a cpu-local variable
tid_t arch_get_current_tid(void) {

    return current_tid;
}


NORETURN void x86_switch_thread(struct arch_thread *thread); // TODO: make it portable
NORETURN void arch_switch_thread(tid_t tid, struct arch_thread *thread) {

    current_tid = tid;
    x86_switch_thread(thread);
    PANIC("x86_switch_thread() returned");
    for (;;); // for supress a compiler warning: 'noreturn' should not return
}


void arch_create_thread(struct arch_thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size) {

#ifdef __APPLE__
    // XXX: Without -8, a SEGV is occurred on macOS.
    //      It seems that this problem is related to the stack
    //      alignment for SSE registers.
    thread->rsp = ((stack + stack_size) & ~0xf) - 8;
#else
    thread->rsp = (stack + stack_size) & ~0xf;
#endif
    
    thread->rip = start;
    thread->rdi = arg;
}
