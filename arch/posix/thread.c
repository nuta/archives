#include "stdlibs.h"
#include <arch.h>
#include <logging.h>
#include <kernel/panic.h>


static struct thread *current_thread; // TODO: make it a cpu-local variable
struct thread *arch_get_current_thread(void) {

    return current_thread;
}


NORETURN void x86_switch_thread(struct arch_thread *thread); // TODO: make it portable
NORETURN void arch_switch_thread(struct thread *thread) {

    current_thread = thread;
    x86_switch_thread(&thread->arch);
    PANIC("x86_switch_thread() returned");
    for (;;); // for supress a compiler warning: 'noreturn' should not return
}


void arch_create_thread(struct thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size) {

#ifdef __APPLE__
    // XXX: Without -8, a SEGV is occurred on macOS.
    //      It seems that this problem is related to the stack
    //      alignment for SSE registers.
    thread->arch.rsp = ((stack + stack_size) & ~0xf) - 8;
#else
    thread->arch.rsp = (stack + stack_size) & ~0xf;
#endif

    thread->arch.rip = start;
    thread->arch.rdi = arg;
}
