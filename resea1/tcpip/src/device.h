#ifndef __TCPIP_DEVICE_H__
#define __TCPIP_DEVICE_H__

#include <resea.h>
#include <resea/tcpip.h>
#include "mbuf.h"

namespace tcpip {

struct net_device {
    channel_t ch;
    void *hwaddr;
    size_t hwaddr_len;
    size_t max_data_size;
    struct addr addr;
    void (*receive)(struct tcpip::net_device *device, void *payload, size_t length);

    // Sends a packet. If `addr` is nullptr, it sends a broadcast packet.
    void (*transmit)(struct tcpip::net_device *device, struct addr* addr,
                     net_type_t protocol, struct tcpip::mbuf *m);
};

void add_device(channel_t ch);
void init_devices();

} // namespace tcpip

#endif
