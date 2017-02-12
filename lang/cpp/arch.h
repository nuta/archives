#ifndef __CPP_ARCH_H__
#define __CPP_ARCH_H__

#include <types.h>
#include <kernel/thread.h>

__EXTERN_C__

struct thread *arch_get_current_thread(void);
NORETURN void arch_switch_thread(struct thread *thread);
void arch_create_thread(struct thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size);
int  arch_yield(struct arch_thread *thread);
void arch_panic(void);
void arch_wait_interrupt(void);
void arch_halt_until(size_t ms);
void arch_printchar(const char ch);
uintmax_t arch_get_cycle_count(void);

__EXTERN_C_END__

#endif
