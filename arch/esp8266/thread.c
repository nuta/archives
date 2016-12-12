#include <arch.h>
#include <logging.h>
#include "thread.h"


static struct thread *current_thread; // TODO: make it a cpu-local variable
struct thread *arch_get_current_thread(void) {

    return current_thread;
}


void arch_switch_thread(struct thread *thread) {

    current_thread = thread;
    INFO("#%d [switch] pc: %p, a1: %p", thread->tid, thread->arch.pc, thread->arch.a1);
    esp8266_switch_thread(&thread->arch);
}


void arch_create_thread(struct thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size) {

    thread->arch.pc = start;
    thread->arch.a0 = 0x00000000; // return address
    thread->arch.a1 = (stack + stack_size) & ~0xf ;
    thread->arch.a2 = arg;
}
