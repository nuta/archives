#include "stdlibs.h"
#include "thread.h"
#include <arch.h>
#include <logging.h>
#include <kernel/panic.h>


static pthread_key_t current_thread;


struct thread *arch_get_current_thread(void) {

    return pthread_getspecific(current_thread);
}


NORETURN void arch_switch_thread(struct thread *thread) {

    for(;;);
}


int arch_yield (struct arch_thread *unused) {

    while (arch_get_current_thread()->state == THREAD_BLOCKED);
    return 1;
}


static void pthread_start(struct thread *thread) {

    pthread_setspecific(current_thread, thread);
    thread->arch.start(thread->arch.arg);
}


void arch_create_thread(struct thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size) {

    thread->arch.start  = (void (*)(void *)) start;
    thread->arch.arg    = (void *) arg;
    pthread_create(&thread->arch.pthread, NULL, (void *(*)(void *)) pthread_start, thread);
}


void posix_init_thread(void) {

    pthread_key_create(&current_thread, NULL);
}
