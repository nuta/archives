#ifndef ___HAL_H__
#define ___HAL_H__

#include <resea.h>

/*
 *  spinlock
 */
#define SPIN_UNLOCKED 0
#define SPIN_LOCKED   1
typedef uint8_t spinlock_t;
typedef uintmax_t irqstatus_t;
typedef uintmax_t cpuid_t;

/* page size is 4KiB */
#define PAGE_SIZE      4096

struct hal_vm_space{
};

struct hal_thread{
};

#endif
