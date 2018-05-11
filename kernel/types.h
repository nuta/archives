#ifndef __TYPES_H__
#define __TYPES_H__

typedef char i8_t;
typedef short i16_t;
typedef int i32_t;
typedef long long i64_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned u32_t;
typedef unsigned long long u64_t;
typedef unsigned char bool;

#define NULL ((void *) 0)
#define false 0
#define true  1
#define STATIC_ASSERT _Static_assert
#define UNUSED __attribute__((unused))
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define DEPRECATED __attribute__((deprecated))
#define UNREACHABLE __builtin_unreachable()
#define ROUND_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define ROUND_DOWN(x, align) ((x) & ~((align) - 1))
#define INLINE_ASM __asm__ __volatile__
#define atomic_compare_and_swap  __sync_bool_compare_and_swap
#define atomic_fetch_and_add  __sync_fetch_and_add
#define atomic_fetch_and_sub  __sync_fetch_and_sub
#define offsetof __builtin_offsetof
#define va_list __builtin_va_list
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define min(x, y)              \
    ({                         \
        __typeof__(x) _x = x;  \
        __typeof__(y) _y = y;  \
        _x > _y ? _y : _x;     \
    })

#include <arch_types.h>
typedef umax_t size_t;
typedef umax_t off_t;

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

#endif
