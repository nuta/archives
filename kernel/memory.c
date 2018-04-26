#include "memory.h"

uintptr_t allocated;
void *kmalloc(size_t size, int flags) {
    void *ptr = (void *) allocated;
    allocated += size;
    return ptr;
}


void memory_init(void) {
    allocated = 0x001000000;
}
