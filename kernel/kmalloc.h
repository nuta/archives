#ifndef __KERNEL_KMALLOC_H__
#define __KERNEL_KMALLOC_H__

#include <types.h>

enum {
    KMALLOC_NORMAL = 0,
};

#define IS_AVAILABLE_CHUNK(chunk) (((uintptr_t) (chunk)->next & 1) == 0)
#define GET_NEXT_CHUNK(chunk_) ((struct chunk *) ((uintptr_t) (chunk_)->next & (~1)))
#define CHUNK_ADDR(chunk_) ((struct chunk *) (((uintptr_t) chunk_) & (~1)))

// The chunk header
struct chunk {
    // The first (0th) bit is used as "being used" bit.
    // We assume that the LSB of pointers is always 0 (aligned).
    struct chunk *next;

    // Size of the usable memory size.
    size_t size;

    // Memory block (data) follows this header.
};


void add_kmalloc_chunk(void *ptr, size_t size);
void *kmalloc(size_t size, int flags);
void kfree(void *ptr);

#endif
