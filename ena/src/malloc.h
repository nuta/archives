#ifndef __ENA_MALLOC_H__
#define __ENA_MALLOC_H__

#include "internal.h"

void *ena_malloc(size_t size);
void *ena_realloc(void *old, size_t size);
void ena_free(void *ptr);

#endif
