#ifndef __ARCH_TYPES_H__
#define __ARCH_TYPES_H__

#include <kernel/prelude.h>

#define PAGE_SIZE 4096
#define PAGE_PRESENT  (1 << 0)
#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER (1 << 2)

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
    return (void *) addr;
}

static inline paddr_t to_paddr(void *addr) {
    return (uptr_t) addr;
}

struct arch_thread {
};

struct arch_vmspace {
};

struct arch_cpuvar {
};


#define CPU_NUM_MAX 32
static inline int posix_get_processor_id(void) {
    return 0;
}

struct cpuvar;
struct cpuvar *cpuvars[CPU_NUM_MAX];
#define CPUVAR (cpuvars[posix_get_processor_id()])

#endif
