#ifndef __POSIX_ARCH_TYPES_H__
#define __POSIX_ARCH_TYPES_H__

#include "stdlibs.h"

#define DEFAULT_THREAD_STACK_SIZE 8192
#define MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
typedef pthread_mutex_t mutex_t;
typedef unsigned long size_t;

struct arch_thread {
    uint64_t  rip;
    uint64_t  rbx;
    uint64_t  rbp;
    uint64_t  rdi;
    uint64_t  rsi;
    uint64_t  rsp;
    uint64_t  r12;
    uint64_t  r13;
    uint64_t  r14;
    uint64_t  r15;
};

#endif
