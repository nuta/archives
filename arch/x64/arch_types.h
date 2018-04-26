#ifndef __ARCH_TYPES_H__
#define __ARCH_TYPES_H__

typedef unsigned long long uintptr_t;
typedef unsigned long long uintmax_t;

struct arch_regs {
};

void arch_init_regs(struct arch_regs *regs);
void arch_switch_to(struct arch_regs *regs);

#endif
