#ifndef __KERNEL_KMALLOC_H__
#define __KERNEL_KMALLOC_H__

#include <types.h>

enum {
    KMALLOC_NORMAL = 0,
};

// The chunk header
struct chunk {
    struct chunk *next;
    size_t size;    // size of an allocation unit without its header
    size_t total;   // # of all allocation units in the chunk
    size_t unused;  // # of unused allocation units in the chunk

    // allocation units follows this header
};

// The allocation unit header
struct alloc {
    int flags;  // 0 on unused or 1 on used

    // memory  block (data) follows this header
};


void add_kmalloc_chunk(void *ptr, size_t size, size_t num);
void *kmalloc(size_t size, int flags);
void kfree(void *ptr);

#endif
