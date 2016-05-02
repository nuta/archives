#ifndef __TCPIP_RECEIVE_H__
#define __TCPIP_RECEIVE_H__

#include "types.h"
#include "instance.h"

void tcpip_receive_packet(struct tcpip_instance *instance, tcpip_link_type_t type,
    const void *payload, size_t size);

#endif
