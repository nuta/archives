#include "stdlibs.h"
#include <kernel/init.h>
#include <kernel/kmalloc.h>
#include <kernel/resources.h>


int main(void) {

    add_kmalloc_chunk(malloc(4096 * 2048), 4096, 1024);
    init_kernel(malloc(sizeof(struct resources)));
}
