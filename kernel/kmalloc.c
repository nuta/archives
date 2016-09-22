#include <mutex.h>
#include <logging.h>
#include "kmalloc.h"

static struct chunk *chunks = NULL;
static mutex_t kmalloc_lock = MUTEX_INITIALIZER;


void add_kmalloc_chunk(void *ptr, size_t size, size_t num) {

    // Initialize the chunk
    // FIXME
    struct chunk *new_chunk = (struct chunk *) ptr;
    new_chunk->size   = size;
    new_chunk->total  = num;
    new_chunk->unused = num;

    // Initialize flags
    struct alloc *alloc = (struct alloc *)
        ((uintptr_t) new_chunk + sizeof(*new_chunk));

    for(size_t i=0; i < new_chunk->total; i++) {
        alloc->flags = 0;

        alloc = (struct alloc *)
            ((uintptr_t) alloc + sizeof(*alloc) + new_chunk->size);
    }

    // insert it into the linked list `chunks`
    // TODO: add lock
    if (!chunks) {
        new_chunk->next = NULL;
        chunks = new_chunk;
    } else {
        struct chunk *chunk = chunks;
        struct chunk *prev_chunk = chunks;
        while (chunk) {
            if (size <= chunk->size) {
                break;
            }

            prev_chunk = chunk;
            chunk = chunk->next;
        }

        if (chunk) {
            new_chunk->next  = chunk;
            prev_chunk->next = new_chunk;
        } else {
            new_chunk->next  = NULL;
            prev_chunk->next = new_chunk;
        }
    }
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

    DEBUG("kmalloc: allocating %d bytes", size);
    mutex_lock(&kmalloc_lock);

    for (struct chunk *chunk = chunks; chunk; chunk = chunk->next) {
        if (size <= chunk->size) {
            struct alloc *alloc = (struct alloc *)
                ((uintptr_t) chunk + sizeof(*chunk));

            for (size_t i=0; i < chunk->total; i++) {
                if (alloc->flags == 0) {
                    // We've found an alloc unit with enough space! Mark it
                    // as being used and return the pointer to the alloc unit.
                    alloc->flags = 1;
                    mutex_unlock(&kmalloc_lock);
                    return (void *) ((uintptr_t) alloc + sizeof(*alloc));
                }

                // try the next alloc
                alloc = (struct alloc *)
                    ((uintptr_t) alloc + sizeof(*alloc) + chunk->size);
            }
        }
    }

    WARN("kmalloc: failed to allocate %d bytes from chunks", size);
    mutex_unlock(&kmalloc_lock);
    return (void *) NULL;
}


void kfree(void *ptr) {

    struct alloc *alloc = (struct alloc *) ((uintptr_t) ptr - sizeof(*alloc));
    alloc->flags = 0;
}
