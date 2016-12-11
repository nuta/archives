#include "stdlibs.h"
#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include <kernel/resources.h>


int main(void) {

    size_t small_chunk_size = 16 * 1024;
    size_t large_chunk_size = 64 * 1024;

    add_kmalloc_chunk(malloc(small_chunk_size), small_chunk_size, false);
    add_kmalloc_chunk(malloc(large_chunk_size), large_chunk_size, true);
    init_kernel(malloc(sizeof(struct resources)));
}
