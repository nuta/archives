#include "stdlibs.h"
#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include <kernel/resources.h>


int main(void) {

    size_t size = 64 * 1024;
    add_kmalloc_chunk(malloc(size), size);
    init_kernel(malloc(sizeof(struct resources)));
}
