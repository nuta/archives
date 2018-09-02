#include "./x64.h"
#include "../port.h"

uintptr_t arch_get_stack_bottom(void) {
    uintptr_t bottom;
    __asm__ __volatile__("movq %%rsp, %0" : "=m"(bottom));
    return bottom;
}

void arch_load_regs(uintptr_t *regs) {
    __asm__ __volatile__(
        "movq %%rax, %0 \n"
        "movq %%rbx, %1 \n"
        "movq %%rcx, %2 \n"
        "movq %%rdx, %3 \n"
        "movq %%rdi, %4 \n"
        "movq %%rsi, %5 \n"
        "movq %%r8,  %6 \n"
        "movq %%r9,  %7 \n"
        "movq %%r10, %8 \n"
        "movq %%r11, %9 \n"
        "movq %%r12, %10 \n"
        "movq %%r13, %11 \n"
        "movq %%r14, %12 \n"
        "movq %%r15, %13 \n"
    :
    "=m"(regs[0]),
    "=m"(regs[1]),
    "=m"(regs[2]),
    "=m"(regs[3]),
    "=m"(regs[4]),
    "=m"(regs[5]),
    "=m"(regs[6]),
    "=m"(regs[7]),
    "=m"(regs[8]),
    "=m"(regs[9]),
    "=m"(regs[10]),
    "=m"(regs[11]),
    "=m"(regs[12]),
    "=m"(regs[13])
    );
}
