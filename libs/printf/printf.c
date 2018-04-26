#include <resea/arch.h>
#include "printf.h"


int printf(const char *fmt, ...) {
    int n = 0;
    while (*fmt != '\0') {
        arch_putchar(*fmt);
        fmt++;
        n++;
    }

    return n;
}
