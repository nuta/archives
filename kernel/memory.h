#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <resea/types.h>

#define KMALLOC_NORMAL 0

void *kmalloc(size_t size, int flags);
void memory_init(void);

#endif
