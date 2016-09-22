#include <arch.h>
#include <logging.h>
#include "thread.h"


static tid_t current_tid; // TODO: make it a cpu-local variable
tid_t arch_get_current_tid(void) {

    return current_tid;
}


void arch_switch_thread(tid_t tid, struct arch_thread *thread) {

    current_tid = tid;
    INFO("#%d [switch] pc: %p, a1: %p", tid, thread->pc, thread->a1);
    esp8266_switch_thread(thread);
}


void arch_create_thread(struct arch_thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size) {

    thread->pc = start;
    thread->a0 = 0x00000000; // return address
    thread->a1 = (stack + stack_size) & ~0xf ;
    thread->a2 = arg;
}
