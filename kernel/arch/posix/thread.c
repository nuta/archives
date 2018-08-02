#include <stdlib.h>
#include <string.h>
#include <kernel/types.h>
#include <kernel/printk.h>

void arch_create_thread(struct arch_thread *arch, bool is_kernel_thread,
                    uptr_t start, umax_t arg,
                    uptr_t stack, size_t stack_size) {

    INFO("stack=%p", stack + stack_size);
    arch->ip = start;
    arch->sp = stack + stack_size;
}

void arch_switch(struct arch_thread *prev, struct arch_thread *next) {
    __asm__ __volatile__(
        "leaq 1f(%%rip), %%rax \n"
        "movq %%rax, %0     \n"
        "movq %%rsp, %1     \n"
        "movq %3, %%rsp     \n"
        "jmpq *%2           \n"
        "1:                 \n"
    : "=m"(prev->ip), "=m"(prev->sp)
    : "r"(next->ip), "r"(next->sp)
    : "%rax"
    );
}
