#ifndef __TCPIP_ETHERNET_H__
#define __TCPIP_ETHERNET_H__

#include "types.h"

namespace tcpip {

struct ethernet_header {
    uint8_t   dest[6];
    uint8_t   src[6];
    uint16_t  type;
} PACKED;

void ethernet_receive(struct net_device *device, void *payload, size_t length);
void ethernet_transmit(struct net_device *device, struct addr* addr,
                       net_type_t protocol, struct mbuf *m);

} // namespace tcpip

#endif
