#include <resea.h>


#ifndef KERNEL

/**
 *  Allocates a memory block
 *
 *  @param[in]  size   The size of memory block to be allocated.
 *  @param[in]  flags  The flags.
 *  @return  The pointer to the allocated memory block on success or
 *           NULL on failure.
 *
 *  @note  In a same executable of core package, this symbol will be overwritten
 *         by core's implementation. Namely, it calls core directly.
 *
 */
void *allocMemory (size_t size, uint32_t flags) {

    WARN("allocMemory() is not implemented yet");
    return NULL;
}


 void freeMemory(void *p){

    /* TODO */
}

#endif
