#include <mutex.h>
#include <logging.h>
#include "kmalloc.h"

static struct chunk *chunks = NULL;
static size_t total = 0;
static size_t used  = 0;
static mutex_t kmalloc_lock = MUTEX_INITIALIZER;


void add_kmalloc_chunk(void *ptr, size_t size) {

    DEBUG("add kmalloc chunk %p (%d bytes)", ptr, size);

    // Initialize the chunk
    struct chunk *new_chunk = (struct chunk *) ptr;
    new_chunk->next = NULL;
    new_chunk->size = size - sizeof(*new_chunk);

    // Insert it into the linked list `chunks`
    mutex_lock(&kmalloc_lock);

    if (!chunks) {
        chunks = new_chunk;
    } else {
        struct chunk *chunk = chunks;
        while (chunk->next) {
            chunk = GET_NEXT_CHUNK(chunk);
        }

        chunk->next = new_chunk;
    }

    total += size;
    mutex_unlock(&kmalloc_lock);
}


/**
 *  Allocates a memory block
 *
 *  @param[in]  size   The size of memory block to be allocated.
 *  @param[in]  flags  Flags.
 *  @return  The pointer to the allocated memory block on success or
 *           NULL on failure.
 *
 */
void *kmalloc(size_t size, int flags) {

    size = ROUND_UP(size, 8);
    DEBUG("kmalloc: allocating %dB (remaining %dB)", size, total - used);
    mutex_lock(&kmalloc_lock);

    for (struct chunk *chunk = chunks; CHUNK_ADDR(chunk); chunk = GET_NEXT_CHUNK(chunk)) {
        if (IS_AVAILABLE_CHUNK(chunk) && size + sizeof(*chunk) <= chunk->size) {
            // We've found an unused chunk with enough space! Split the memory
            // block, mark it as being used and return the pointer to the
            // memory block.
            struct chunk *next_chunk = (void *) ((uintptr_t) chunk +
                                                 sizeof(*chunk) + size);
            next_chunk->next = chunk->next;
            next_chunk->size = chunk->size - sizeof(*chunk) - size;
            chunk->next      = (void *) ((uintptr_t) next_chunk | 1);
            chunk->size      = size;
            used += size;

            mutex_unlock(&kmalloc_lock);
            return (void *) ((uintptr_t) chunk + sizeof(*chunk));
        }
    }

    WARN("kmalloc: failed to allocate %d bytes from chunks", size);
    mutex_unlock(&kmalloc_lock);
    return (void *) NULL;
}


void kfree(void *ptr) {
    struct chunk *free_chunk = (struct chunk *) ((uintptr_t) ptr - sizeof(*free_chunk));

    mutex_lock(&kmalloc_lock);
    free_chunk->next = (void *) ((uintptr_t) free_chunk->next & (~1));
    used -= free_chunk->size;

    // Merge continuous free chunks as much as possible.
    for (struct chunk *c = chunks; c; c = GET_NEXT_CHUNK(c)) {
        struct chunk *c_tmp = c;

        if (IS_AVAILABLE_CHUNK(c)) {
            struct chunk *next_chunk = GET_NEXT_CHUNK(c);
            while (CHUNK_ADDR(next_chunk) && IS_AVAILABLE_CHUNK(next_chunk) &&
                   (c + sizeof(*c) + c->size) == CHUNK_ADDR(next_chunk)) {

                c->size = next_chunk->size + sizeof(*c);
                c->next = next_chunk->next;
                next_chunk = GET_NEXT_CHUNK(c);
            }
        }

        c = c_tmp;
    }

    mutex_unlock(&kmalloc_lock);
}


size_t get_remaining_memory(void) {
    return total - used;
}
