#ifndef __RESEA_TYPES_H__
#define __RESEA_TYPES_H__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned uint32_t;
typedef unsigned long long uint64_t;

#define UNUSED __attribute__((unused))

#include <resea/arch.h>

typedef uintmax_t size_t;

#define INLINE_ASM __asm__ __volatile__

#endif
