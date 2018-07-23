#ifndef __TYPES_H__
#define __TYPES_H__

#include <arch_types.h>

struct cpuvar {
    struct arch_cpuvar arch;
    struct thread *current;
    struct thread *idle_thread;
    struct runqueue *runqueue;
    kmutex_t runqueue_lock;
};

struct page {
    int ref_count;
};

// 0 - NULL_PAGE_SIZE are not allocated for both kernel and user. Pointers points
// to the range as invalid.
#define NULL_PAGE_SIZE (0x1000)
#define ERROR_PTR(error) ((void *) (error))
#define IS_ERROR_PTR(ptr) ((uptr_t) (ptr) < NULL_PAGE_SIZE)
#define ERROR_FROM_PTR(ptr) ((int) (ptr))

void kernel_init(void);

// Arch is responsible for call this every 1/TICK_HZ sec.
void tick_timer(void);

// Arch is responsible for call this function on an interrupt.
void handle_irq(int irq);

void arch_init(void);
void arch_early_init(void);
void arch_putchar(char ch);
NORETURN void arch_panic(void);
void arch_idle(void);
void arch_create_thread(struct arch_thread *arch, bool is_kernel_thread,
                    uptr_t start, umax_t arg,
                    uptr_t stack, size_t stack_size);
void arch_destroy_thread(struct arch_thread *arch);
void arch_switch(struct arch_thread *prev, struct arch_thread *next);

void arch_create_vmspace(struct arch_vmspace *vms);
void arch_destroy_vmspace(struct arch_vmspace *vms);
void arch_switch_vmspace(struct arch_vmspace *vms);
void arch_link_page(struct arch_vmspace *vms, uptr_t vaddr, paddr_t paddr, size_t num,
                    int attrs);
void arch_copy_from_user(void *kernel, uptr_t user, size_t length);
void arch_allow_io(struct arch_thread *arch);
void arch_accept_irq(int irq);

extern paddr_t phypages_start;
extern size_t phypages_num;

#endif
