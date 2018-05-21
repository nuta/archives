#ifndef __CPP_TYPES_H__
#define __CPP_TYPES_H__

#include <stdarg.h>

enum {
    OK               = 1,
    E_BAD_CID        = 2,
    E_ALREADY_RECVED = 3,
    E_NOT_LINKED     = 4,
    E_BAD_MEMCPY     = 5,
    E_NOMEM          = 6,
    E_BAD_REQUEST    = 7
};

#define PACKED      __attribute__((packed))
#define NORETURN    __attribute__((noreturn))
#define UNUSED      __attribute__((unused))
#define DEPRECATED  __attribute__((deprecated))
#define UNREACHABLE __builtin_unreachable()

#define ROUND_UP(x, len) (((x) + (len - 1)) & ~(len - 1))

#ifdef __cplusplus
#define __EXTERN_C__      extern "C" {
#define __EXTERN_C_END__  }
#else
#define __EXTERN_C__
#define __EXTERN_C_END__
#endif

#ifdef ARCH_POSIX

#include "arch/posix/stdlibs.h"
typedef pid_t  tid_t;

#else // !ARCH_POSIX

typedef char        int8_t;
typedef short       int16_t;
typedef int         int32_t;
typedef long long   int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

#ifdef __LP64__
typedef int64_t   intmax_t;
typedef uint64_t  uintmax_t;
#else // !__LP64__
typedef int32_t   intmax_t;
typedef uint32_t  uintmax_t;
#endif // __LP64__

#ifndef __cplusplus
typedef uint8_t bool;
#define true  1
#define false 0
#endif

#define NULL ((void *) 0)

typedef uintmax_t  uintptr_t;
typedef uintmax_t  tid_t;
typedef tid_t      pid_t;

#endif // ARCH_POSIX

typedef uint8_t    result_t;
typedef uintmax_t  payload_t;
typedef uintmax_t  cid_t;

#include <arch_types.h>

#endif
