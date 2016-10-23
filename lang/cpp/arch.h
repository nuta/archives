#ifndef __CPP_ARCH_H__
#define __CPP_ARCH_H__

#include <types.h>

tid_t arch_get_current_tid(void);
NORETURN void arch_switch_thread(tid_t tid, struct arch_thread *thread);
void arch_create_thread(struct arch_thread *thread, uintptr_t start, uintmax_t arg,
                        uintptr_t stack, size_t stack_size);
int  arch_yield(struct arch_thread *thread);
void arch_panic(void);
void arch_wait_interrupt(void);
void arch_printchar(const char ch);

#endif
