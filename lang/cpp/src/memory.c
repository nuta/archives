#include <resea.h>
#include <resea/channel.h>
#include <resea/memory.h>


static channel_t memory_ch = 0;

/**
 *  Allocates a memory block
 *
 *  @param[in]  size   The size of memory block to be allocated.
 *  @param[in]  flags  Flags.
 *  @return  The pointer to the allocated memory block on success or
 *           NULL on failure.
 *
 */
void *allocate_memory (size_t size, uint32_t flags) {
    result_t r;
    uintptr_t p;

    if (!memory_ch) {
        memory_ch = sys_open();
        call_channel_connect(connect_to_local(1), memory_ch,
            INTERFACE(memory), &r);
    }

    call_memory_allocate(memory_ch, size, flags, &r, &p);
    return (r == OK)? (void *) p : NULL;
}


 void release_memory(void *p){

    /* TODO */
}
