#ifndef __STRFMT_H__
#define __STRFMT_H__

#include <stdarg.h>

void strfmt(const char *fmt, void (*callback)(void *arg, char c),
            void *callback_arg, ...);
void vstrfmt(const char *fmt, va_list vargs, void (*callback)(void *arg, char c),
             void *callback_arg);

#endif