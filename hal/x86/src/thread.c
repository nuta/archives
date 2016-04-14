#include <hal.h>
#include <resea/cpp/memory.h>
#include <string.h>


void hal_set_current_thread_id(ident_t id) {

    CPUVAR->current_thread = id;
}


ident_t hal_get_current_thread_id(void) {

    return CPUVAR->current_thread;
}


void hal_set_thread(struct hal_thread *t, bool is_kernel,
                    uintptr_t entry, uintptr_t arg, uintptr_t stack, uintptr_t stack_size) {

    t->is_kernel = is_kernel;
    t->regs.cs     = KERNEL_CODE64_SEG;
    t->regs.ss     = KERNEL_DATA64_SEG;
    t->regs.rflags = 0x202;
    t->regs.rsp    = (uint64_t) stack + stack_size;
    t->regs.rip    = (uint64_t) entry;
    t->regs.rdi    = (uint64_t) arg; // the 1st argument
    t->fregs       = kernel_allocate_memory(XSAVE_AREA_SIZE, MEMORY_ALLOC_PAGE_ALIGNED);

    if (!is_kernel) {
        t->regs.cs = USER_CODE64_SEG | 3;
        t->regs.ss = USER_DATA64_SEG | 3;
    }
}


NORETURN void hal_resume_thread(ident_t id, struct hal_thread *t, struct hal_vm_space *vm) {

    hal_set_current_thread_id(id);
    hal_switch_vm_space(vm);
    x86_asm_fxrstor(t->fregs);
    x86_asm_resume_thread(t->is_kernel, &t->regs);
}


void x86_soft_save_thread (struct hal_thread *t,
                           uint64_t rsp, uint64_t rdi, uint64_t rsi,
                           uint64_t rbx, uint64_t rbp, uint64_t r12,
                           uint64_t r13, uint64_t r14, uint64_t r15){

  t->regs.rip    = (uint64_t) x86_unblocked_thread_entry;
  t->regs.rsp    = rsp;
  t->regs.rdi    = rdi;
  t->regs.rsi    = rsi;
  t->regs.rbx    = rbx;
  t->regs.rbp    = rbp;
  t->regs.r12    = r12;
  t->regs.r13    = r13;
  t->regs.r14    = r14;
  t->regs.r15    = r15;
  t->regs.cs     = KERNEL_CODE64_SEG;
  t->regs.ss     = KERNEL_DATA64_SEG;
}

/**
 *  save a thread status (CPU registers)
 *
 *  @param[in] t  thread
 *  @warning  It MUST be called in an interrupt context.
 *
 */
void hal_save_thread(struct hal_thread *t) {
    struct x86_thread_regs *regs;

    x86_asm_fxsave(t->fregs);

    /* registers are stored in the stack for interrupt handlers */
    regs = (struct x86_thread_regs *) ((uintptr_t) CPUVAR->intr_stack_addr -
                                       sizeof(struct x86_thread_regs));
    memcpy(&t->regs, regs, sizeof(struct x86_thread_regs));

    /* segment registers will be overwritten by x86_soft_save_thread() */
    if (!t->is_kernel) {
        t->regs.cs = USER_CODE64_SEG | 3;
        t->regs.ss = USER_DATA64_SEG | 3;
    }
}
