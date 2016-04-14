#ifndef __CPP_MEMORY_H__
#define __CPP_MEMORY_H__

#include <resea/memory.h>

void *allocMemory(size_t size, memory_allocmem_t flags);
void freeMemory(void *p);
void *allocPhysicalMemory(paddr_t addr, size_t size, uint32_t flags, paddr_t *alloced_addr); // TODO: remove

#endif
