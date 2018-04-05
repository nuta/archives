#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

void logger(int level, const char *prefix, const char *format, ...) {
    va_list vargs;
    va_start(vargs, format);

    printf("[%s] ", prefix);
    vprintf(format, vargs);
    printf("\n");

    va_end(vargs);
}
