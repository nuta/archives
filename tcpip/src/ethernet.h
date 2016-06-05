#ifndef __TCPIP_ETHERNET_H__
#define __TCPIP_ETHERNET_H__

#include "types.h"

struct ethernet_header {
    uint8_t   dest[6];
    uint8_t   src[6];
    uint16_t  type;
} PACKED;

void tcpip_ethernet_transmit(struct net_device *device, struct addr* addr,
                             tcpip_protocol_t protocol, struct mbuf *m);

#endif
