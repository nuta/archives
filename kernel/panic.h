#ifndef __KERNEL_PANIC_H__
#define __KERNEL_PANIC_H__

#include <logging.h>
#include <arch.h>

#define PANIC(fmt, ...)  do { \
        printfmt("P> "  fmt, ##__VA_ARGS__); \
        arch_panic();                        \
    } while(0)

#endif
