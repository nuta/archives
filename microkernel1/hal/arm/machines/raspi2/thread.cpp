#include "../../src/arm.h"
#include <hal.h>
#include <string.h>


void hal_set_thread(struct hal_thread *t, bool is_kernel,
                    uintptr_t entry, uintptr_t arg, uintptr_t stack, uintptr_t stack_size) {

    BUG_IF(!is_kernel, "ARM raspi2 HAL does not support user mode");
    t->is_kernel = is_kernel;
    t->regs.spsr = (is_kernel)? 0x13 : 0x10;
    t->regs.r0   = arg;
    t->regs.r1   = 0;
    t->regs.r2   = 0;
    t->regs.r3   = 0;
    t->regs.r4   = 0;
    t->regs.r5   = 0;
    t->regs.r6   = 0;
    t->regs.r7   = 0;
    t->regs.r8   = 0;
    t->regs.r9   = 0;
    t->regs.r10  = 0;
    t->regs.r11  = 0;
    t->regs.r12  = 0;
    t->regs.sp   = stack + stack_size;
    t->regs.lr   = 0;
    t->regs.pc   = entry;
}


void hal_resume_thread(ident_t id, struct hal_thread *t, struct hal_vm_space *vms) {

    // copy registers except sp and spsr into the thread's stack
    size_t length = sizeof(t->regs) - 8;
    t->regs.sp -= length;

    // XXX: verify that the stack is accessible
    memcpy((void *) t->regs.sp, (const void *) &t->regs, length);

    arm_asm_resume_thread(t->regs.sp, t->regs.spsr);
}


// Must be called in the IRQ handling.
void hal_save_thread(struct hal_thread *t) {

    memcpy(&t->regs, CPUVAR->irq_thread_state,
           sizeof(t->regs));
}


void hal_switch_thread(struct hal_thread *t) {

    WARN("hal_switch_thread() is not implemented");
}
