#include "malloc.h"

// TODO: Implement our own memory allocator and remove
// these "#include"s.
#include <assert.h>
#include <stdlib.h>

void *ena_malloc(size_t size) {
    void *ptr = malloc(size);
    assert(ptr);
    return ptr;
}

void *ena_realloc(void *old, size_t size) {
   void *ptr = realloc(old, size);
    assert(ptr);
    return ptr;
}

void ena_free(void *ptr) {
    free(ptr);
}
