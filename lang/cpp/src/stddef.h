#ifndef __C_STDDEF_H__
#define __C_STDDEF_H__

#ifdef POSIX_HOST

#include_next <stddef.h>

#else

#include <stdint.h>
typedef uintmax_t size_t; 

#endif

#endif
