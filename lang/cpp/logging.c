#include <stdarg.h>
#include <arch.h>
#include <mutex.h>
#include <strfmt.h>
#include <malloc.h>
#include "logging.h"

static char *buffer = NULL;
static size_t buffer_start = 0, buffer_end = 0, buffer_filled = 0;


static void printchar(const char ch) {

    arch_printchar(ch);

    if (!buffer)
        return;

    if (ch == '\n')
        buffer_filled = buffer_end;

    buffer[buffer_end] = ch;
    buffer_end++;

    if (buffer_end == BUFFER_SIZE - 1)
        buffer_end = 0;
}


size_t get_buffered_log(char **s) {

    if (buffer_filled < buffer_start) {
        *s = &buffer[buffer_start];
        if (buffer_end < buffer_start) {
            // wraped
            size_t start = buffer_start;
            buffer_start = 0;
            return BUFFER_SIZE - start + 1;
        } else {
            // nothing to return
            return 0;
        }
    }

    size_t start = buffer_start;
    buffer_start = buffer_filled + 1;

    *s = &buffer[start];
    return buffer_filled - start;
}


static mutex_t printfmt_lock = MUTEX_INITIALIZER;


static void vstrfmt_printchar(void *unused, char c) {

    printchar(c);
}


void vprintfmt(const char *format, va_list vargs) {

    vstrfmt(format, vargs, vstrfmt_printchar, NULL);
}


void printfmt(const char *format, ...) {
    va_list vargs;

    mutex_lock(&printfmt_lock);

    va_start(vargs, format);
    vprintfmt(format, vargs);
    va_end(vargs);

    printchar('\n');

    mutex_unlock(&printfmt_lock);
}


void printfmt_nonl(const char *format, ...) {
    va_list vargs;

    mutex_lock(&printfmt_lock);

    va_start(vargs, format);
    vprintfmt(format, vargs);
    va_end(vargs);

    mutex_unlock(&printfmt_lock);
}


void init_logging() {

    buffer = malloc(BUFFER_SIZE);
}
