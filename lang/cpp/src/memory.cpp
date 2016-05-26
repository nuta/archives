#include "cpp.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/cpp/memory.h>


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
    uintmax_t flags;  // 0 on unused or 1 on used
    uintptr_t callee; // the callee address TODO: disable it if TEST == true

    // memory block for callee follows this header
};


static struct chunk *chunks = nullptr;
static mutex_t lock = MUTEX_UNLOCKED;

static void add_chunk(size_t size, size_t num) {
    result_t r;
    uintptr_t p;

    call_memory_allocate(get_memory_ch(),
        sizeof(struct chunk) + (sizeof(struct alloc) + size) * num,
        MEMORY_ALLOC_NORMAL,
        &r, &p);

    if (r != OK) {
        WARN("failed to add malloc chunk");
        return;
    }

    INFO("adding a malloc chunk: %p size=%u, num=%u", p, size, num);

    // initialize the chunk
    struct chunk *new_chunk = (struct chunk *) p;
    new_chunk->size   = size;
    new_chunk->total  = num;
    new_chunk->unused = num;

    // insert it into the linked list `chunks`
    // TODO: add lock
    if (!chunks) {
        new_chunk->next = nullptr;
        chunks = new_chunk;
    } else {
        struct chunk *chunk = chunks;
        while (chunk && chunk->next) {
            if (chunk->size <= size) {
                break;
            }

            chunk = chunk->next;
        } 

        new_chunk->next = chunk->next;
        chunk->next     = new_chunk;
    }
}


channel_t get_memory_ch(void) {
    static channel_t ch = 0;
    result_t r;

    if (!ch) {
        ch = create_channel();
        call_channel_connect(connect_to_local(1), ch,
            INTERFACE(memory), &r);
    }

    return ch;
}


/**
 *  Allocates a memory block
 *
 *  @param[in]  size   The size of memory block to be allocated.
 *  @param[in]  flags  Flags.
 *  @return  The pointer to the allocated memory block on success or
 *           nullptr on failure.
 *
 */
void *allocate_memory (size_t size, memory_alloc_t flags) {

    lock_mutex(&lock);

    if (!chunks) {
        // TODO: add chunk dynamically
        add_chunk(8,    4098);
        add_chunk(64,   4098);
        add_chunk(256,  4098);
        add_chunk(512,  4098);
        add_chunk(1024, 4098);
        add_chunk(4096, 4098);
        add_chunk(1024 * 1024, 32);
    }

    for (struct chunk *chunk = chunks; chunk; chunk = chunk->next) {
        if (size <= chunk->size && chunk->unused > 0) {
            struct alloc *alloc = (struct alloc *)
                ((uintptr_t) chunk + sizeof(*chunk));

            for(int i=0; i < chunk->total; i++) {

                if (!(alloc->flags & 1)) {
                    // unused unit, try to lock
                    if (COMPARE_AND_SWAP(&alloc->flags, 0, 1)) {
                        // OK
                        ATOMIC_SUB(&chunk->unused, 1);
                        alloc->callee = RETURN_ADDRESS;
                        unlock_mutex(&lock);
                        return (void *) ((uintptr_t) alloc + sizeof(*alloc));
                    }
                }

                alloc = (struct alloc *)
                    ((uintptr_t) alloc + sizeof(*alloc) + chunk->size);
            }
        }
    }

    unlock_mutex(&lock);
    WARN("failed to allocate memory: size=%d", size);
    return nullptr;
}


void release_memory(void *p){

    /* TODO */
}


void report_unfreed_memory() {

    for (struct chunk *chunk = chunks; chunk; chunk = chunk->next) {
        struct alloc *alloc = (struct alloc *)
            ((uintptr_t) chunk + sizeof(*chunk));

        for(int i=0; i < chunk->total; i++) {

            if (alloc->flags & 1) {
                BUG("unfreed memory allocated at: %P", alloc->callee);
            }

            alloc = (struct alloc *)
                ((uintptr_t) alloc + sizeof(*alloc) + chunk->size);
        }
    }
}
