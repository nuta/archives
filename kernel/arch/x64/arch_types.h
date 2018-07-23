#ifndef __ARCH_TYPES_H__
#define __ARCH_TYPES_H__

#include <kernel/prelude.h>

#define GDT_DESC_NUM 8
#define IDT_DESC_NUM    256

struct seg_desc {
    u16_t limit1;
    u16_t base1;
    u8_t  base2;
    u8_t  type;
    u8_t  limit2;
    u8_t  base3;
} PACKED;

struct tss_desc {
    u16_t limit1;
    u16_t base1;
    u8_t  base2;
    u8_t  type;
    u8_t  limit2;
    u8_t  base3;
    u32_t base4;
    u32_t reserved;
} PACKED;

struct gdtr {
    u16_t length;
    u64_t address;
} PACKED;

struct intr_desc {
    u16_t offset1;
    u16_t seg;
    u8_t  ist;
    u8_t  info;
    u16_t offset2;
    u32_t offset3;
    u32_t reserved;
} PACKED;

struct idtr {
    u16_t length;
    u64_t address;
} PACKED;

struct tss {
    u32_t  reserved0;
    u64_t  rsp0;
    u64_t  rsp1;
    u64_t  rsp2;
    u64_t  reserved1;
    u64_t  ist[7];
    u64_t  reserved2;
    u16_t  reserved3;
    u16_t  iomap;
} PACKED;

// For debugging on Bochs.
#define MAGICBREAK __asm__ __volatile__("xchgw %bx,%bx");

#define KERNEL_BASE_ADDR 0xffff800000000000
#define PAGE_SIZE 4096
#define PAGE_PRESENT  (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)

#define TICK_HZ 1000 /* Fires tick_timer() every 1/TICK_HZ sec. */

/* TODO: lock-free! */
#define KMUTEX_UNLOCKED 0
#define KMUTEX_LOCKED 1
typedef u32_t kmutex_t;
typedef u64_t kmutex_state_t;

static inline void kmutex_init(kmutex_t *lock, int init) {
    *lock = init;
}

static inline kmutex_state_t kmutex_lock_irq_disabled(kmutex_t *lock) {
    kmutex_state_t state;

    INLINE_ASM(
        "1:                        \n"
        "    pause                 \n"
        "    lock cmpxchgl %1, %2  \n"
        "    jnz 1b                \n"
        "    pushfq                \n "
        "    pop %%rax             \n"
    : "=a"(state)
    : "r"(KMUTEX_UNLOCKED), "m"(lock)
    );

    return state;
}

static inline void kmutex_unlock_restore_irq(kmutex_t *lock, kmutex_state_t state) {
    INLINE_ASM(
        "push $2      \n"
        "popfq        \n"
        "movl %1, %0  \n"
    : "=m"(lock)
    : "r"(KMUTEX_UNLOCKED), "r"(state)
    );
}

static inline void *from_paddr(paddr_t addr) {
    return (void *) (addr + KERNEL_BASE_ADDR);
}

static inline paddr_t to_paddr(void *addr) {
    return ((uptr_t) addr - KERNEL_BASE_ADDR);
}

#include "arch_thread.h"

struct arch_vmspace {
    paddr_t pml4_addr;
};

struct arch_cpuvar {
    struct seg_desc gdt[GDT_DESC_NUM];
    struct intr_desc idt[IDT_DESC_NUM];
    struct gdtr gdtr;
    struct idtr idtr;
    struct tss tss;
};


#define CPU_NUM_MAX 32
static inline int x64_get_processor_id(void) {
    return *((volatile u32_t *) from_paddr(0xfee00020)) >> 24;
}

struct cpuvar;
struct cpuvar *cpuvars[CPU_NUM_MAX];
#define CPUVAR (cpuvars[x64_get_processor_id()])

#endif
