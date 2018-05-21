#ifndef __POSIX_ARCH_TYPES_H__
#define __POSIX_ARCH_TYPES_H__

#include "stdlibs.h"

#define DEFAULT_THREAD_STACK_SIZE (1024 * 16)
#define MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
typedef pthread_mutex_t mutex_t;
typedef unsigned long size_t;

struct arch_thread {
    pthread_t pthread;
    void (*start)(void*);
    void *arg;
};

#endif
