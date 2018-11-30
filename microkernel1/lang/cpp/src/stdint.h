#ifndef __C_STDINT_H__
#define __C_STDINT_H__

#ifdef POSIX_HOST

#include_next <stdint.h>

#else

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned        uint32_t;
typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed          int32_t;
typedef unsigned long long  uint64_t;
typedef signed long long    int64_t;

#ifdef __LP64__
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
#else // !__LP64__
typedef int32_t intmax_t;
typedef uint32_t uintmax_t;
#endif


typedef uintmax_t uintptr_t;

#endif // POSIX_HOST

#endif
