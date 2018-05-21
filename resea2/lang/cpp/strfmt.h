#ifndef __STRFMT_H__
#define __STRFMT_H__

#include <types.h>

__EXTERN_C__


void strfmt(const char *fmt, void (*callback)(void *arg, char c),
            void *callback_arg, ...);
void vstrfmt(const char *fmt, va_list vargs, void (*callback)(void *arg, char c),
             void *callback_arg);


__EXTERN_C_END__
#endif
