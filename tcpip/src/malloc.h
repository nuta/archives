#ifndef __TCPIP_MALLOC_H__
#define __TCPIP_MALLOC_H__

#include "types.h"

void *tcpip_malloc(size_t size);
void tcpip_free(void *ptr);

#endif
