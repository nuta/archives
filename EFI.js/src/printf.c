#include <stddef.h>
#include "strfmt.h"


static void vstrfmt_printchar(void *unused, char c) {

    printchar(c);
}


void vprintf(const char *format, va_list vargs) {

    vstrfmt(format, vargs, vstrfmt_printchar, NULL);
}


void printf(const char *format, ...) {
    va_list vargs;

    va_start(vargs, format);
    vprintf(format, vargs);
    va_end(vargs);

    printchar('\n');
}
