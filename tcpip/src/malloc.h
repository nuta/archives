#ifndef __TCPIP_MALLOC_H__
#define __TCPIP_MALLOC_H__

#include "types.h"
#include "instance.h"

void *tcpip_malloc(struct tcpip_instance *instance, size_t size);
void tcpip_free(struct tcpip_instance *instance, void *ptr);

#endif
