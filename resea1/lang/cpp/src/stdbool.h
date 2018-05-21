#ifndef __C_STDBOOL_H__
#define __C_STDBOOL_H__

#ifndef __cplusplus

#ifdef POSIX_HOST

#include_next <stdbool.h>

#else

#include <stdint.h>

typedef uint8_t bool;
#define true  1
#define false 0

#endif // POSIX_HOST
#endif // __cplusplus

#endif

