#ifndef __CPP_MALLOC_H__
#define __CPP_MALLOC_H__

#ifndef ARCH_POSIX
#ifndef KERNEL

#include <kernel/kmalloc.h>


static inline void *malloc(size_t size) {

    return kmalloc(size, KMALLOC_NORMAL);
}


static inline void free(void *ptr) {

    kfree(ptr);
}

#else // !KERNEL

#error "malloc in userspace is not yet implemented"

#endif // KERNEL
#endif // ARCH_POSIX

#endif
