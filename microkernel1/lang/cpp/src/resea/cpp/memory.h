#ifndef __CPP_MEMORY_H__
#define __CPP_MEMORY_H__

#include <resea/memory.h>

channel_t get_memory_ch(void);
void *allocate_memory(size_t size, resea::interfaces::memory::alloc_t flags);
void release_memory(void *p);
void report_unfreed_memory();

#endif
