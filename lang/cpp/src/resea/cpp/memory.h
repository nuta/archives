#ifndef __CPP_MEMORY_H__
#define __CPP_MEMORY_H__

#include <resea/memory.h>

void *allocate_memory(size_t size, memory_allocmem_t flags);
void release_memory(void *p);
void *allocPhysicalMemory(paddr_t addr, size_t size, uint32_t flags, paddr_t *alloced_addr); // TODO: remove

#endif
