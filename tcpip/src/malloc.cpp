#include <resea.h>
#include <resea/cpp/memory.h>
#include "malloc.h"


void *tcpip_malloc(struct tcpip_instance *instance, size_t size) {

    // TODO
    return allocate_memory(size, MEMORY_ALLOC_NORMAL);
}


void tcpip_free(struct tcpip_instance *instance, void *ptr) {

    // TODO
    return release_memory(ptr);
}
