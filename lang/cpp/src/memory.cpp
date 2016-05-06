#include <resea.h>
#include <resea/channel.h>
#include <resea/memory.h>


channel_t get_memory_ch(void) {
    static channel_t ch = 0;
    result_t r;

    if (!ch) {
        ch = sys_open();
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
    result_t r;
    uintptr_t p;

    call_memory_allocate(get_memory_ch(), size, flags, &r, &p);
    return (r == OK)? (void *) p : nullptr;
}


 void release_memory(void *p){

    /* TODO */
}
