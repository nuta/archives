#include "stdlibs.h"
#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include <kernel/resources.h>


int main(void) {

    size_t small_chunk_size = 16 * 1024;
    size_t large_chunk_size = 64 * 1024;

    void *small_chunk = malloc(small_chunk_size);
    void *large_chunk = malloc(large_chunk_size);

    if (!small_chunk || !large_chunk) {
        fprintf(stderr, "failed to malloc(3)\n");
        exit(1);
    }

    add_kmalloc_chunk(small_chunk, small_chunk_size, false);
    add_kmalloc_chunk(large_chunk, large_chunk_size, true);
    init_kernel(malloc(sizeof(struct resources)));
}
