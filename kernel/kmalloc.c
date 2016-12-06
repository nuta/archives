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

    for (struct chunk *chunk = chunks; chunk; chunk = GET_NEXT_CHUNK(chunk)) {
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

    struct chunk *chunk = (struct chunk *) ((uintptr_t) ptr - sizeof(*chunk));

    mutex_lock(&kmalloc_lock);
    used -= chunk->size;
    mutex_unlock(&kmalloc_lock);

    chunk->next = (void *) ((uintptr_t) chunk->next & (~1));
}
