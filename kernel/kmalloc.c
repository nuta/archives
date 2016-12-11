#include <mutex.h>
#include <logging.h>
#include "kmalloc.h"

static struct chunk *large_chunks = NULL;
static struct chunk *small_chunks = NULL;
static size_t total = 0;
static size_t used  = 0;
static mutex_t kmalloc_lock = MUTEX_INITIALIZER;


void add_kmalloc_chunk(void *ptr, size_t size, bool large) {

    INFO("kmalloc: add chunk %dB", ptr, size);

    // Initialize the chunk
    struct chunk *new_chunk = (struct chunk *) ptr;
    new_chunk->next = NULL;
    new_chunk->size = size - sizeof(*new_chunk);

    // Insert it into the linked list `chunks`
    mutex_lock(&kmalloc_lock);

    struct chunk **chunks = (large)? &large_chunks : &small_chunks;
    if (!*chunks) {
        *chunks = new_chunk;
    } else {
        struct chunk *chunk = *chunks;
        while (chunk->next) {
            chunk = GET_NEXT_CHUNK(chunk);
        }

        chunk->next = new_chunk;
    }

    total += size;
    mutex_unlock(&kmalloc_lock);
}


static void *_kmalloc(struct chunk *chunks, size_t size, int flags) {

    size = ROUND_UP(size, 8);
    mutex_lock(&kmalloc_lock);

    for (struct chunk *chunk = chunks; CHUNK_ADDR(chunk); chunk = GET_NEXT_CHUNK(chunk)) {
        if (IS_AVAILABLE_CHUNK(chunk) && size <= chunk->size) {
            // We've found an unused chunk with enough space!
            if (size + sizeof(*chunk) < chunk->size) {
                // Split the memory block.
                struct chunk *next_chunk = (void *) ((uintptr_t) chunk +
                                                     sizeof(*chunk) + size);
                next_chunk->next = chunk->next;
                next_chunk->size = chunk->size - sizeof(*chunk) - size;
                chunk->next      = next_chunk;
            }

            // Mark as being used.
            chunk->next = (void *) ((uintptr_t) chunk->next | 1);
            chunk->size = size;
            used += size;
            INFO("kmalloc: allocate %dB", size);
            mutex_unlock(&kmalloc_lock);
            return (void *) ((uintptr_t) chunk + sizeof(*chunk));
        }
    }

    WARN("kmalloc: failed to allcoate %dB", size);
    mutex_unlock(&kmalloc_lock);
    return (void *) NULL;
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
    struct chunk *chunks;

    chunks = (IS_WITHIN_LARGE_CHUNK_SIZE(size))? large_chunks : small_chunks;
    return _kmalloc(chunks, size, flags);
}


/* Merges continuous free chunks as much as possible.  Caller must
   lock `kmalloc_lock` */
static void merge_cont_chunks(struct chunk *chunks) {

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
}

void kfree(void *ptr) {
    struct chunk *free_chunk = GET_CHUNK_BY_PTR(ptr);

    mutex_lock(&kmalloc_lock);

    free_chunk->next = (void *) ((uintptr_t) free_chunk->next & (~1));
    used -= free_chunk->size;
    merge_cont_chunks(small_chunks);
    merge_cont_chunks(large_chunks);

    mutex_unlock(&kmalloc_lock);
}


static bool is_valid_pointer(struct chunk *chunks, void *ptr) {
    struct chunk *ptr_chunk = GET_CHUNK_BY_PTR(ptr);

    for (struct chunk *c = chunks; c; c = GET_NEXT_CHUNK(c)) {
        if (c == ptr_chunk)
            return true;
    }

    return false;
}


/* Try to free the memory space. `ptr` can be invalid pointer. */
void try_kfree(void *ptr) {

    if (is_valid_pointer(large_chunks, ptr) || is_valid_pointer(small_chunks, ptr))
        kfree(ptr);
}


size_t get_remaining_memory(void) {
    return total - used;
}
