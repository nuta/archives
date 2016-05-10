#ifndef __TCPIP_RECEIVE_H__
#define __TCPIP_RECEIVE_H__

#include "types.h"

void tcpip_receive_packet(tcpip_link_type_t type,
    const void *payload, size_t size);

#endif
