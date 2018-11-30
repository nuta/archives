#ifndef __C_STDARG_H__
#define __C_STDARG_H__

#ifdef POSIX_HOST

#include_next <stdarg.h>

#else

typedef __builtin_va_list va_list;
#define va_start(v, l)      __builtin_va_start(v, l)
#define va_end(v)           __builtin_va_end(v)
#define va_arg(v, l)        __builtin_va_arg(v, l)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)

#endif

#endif
