#include "tcpip.h"
#include "arp.h"
#include "device.h"
#include "ethernet.h"
#include <string.h>
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/tcpip.h>
#include <resea/net_device.h>


static uint16_t protocol_to_ethtype(tcpip_protocol_t protocol) {

    if (protocol & TCPIP_PROTOCOL_IPV4) {
        return 0x0800;
    } else if (protocol & TCPIP_PROTOCOL_IPV6) {
        return 0x86dd;
    } else if (protocol & TCPIP_PROTOCOL_ARP) {
        return 0x0806;
    }

    BUG("unknown protocol type: %#x", protocol);
    return 0;
}


void tcpip_ethernet_transmit(struct net_device *device, struct addr* addr,
                             tcpip_protocol_t protocol, struct mbuf *m) {

    size_t data_len;
    void *data, *payload;
    struct ethernet_header *header;

    data_len = m->total_length;
    data = allocate_memory(data_len + sizeof(struct ethernet_header),
                           MEMORY_ALLOC_NORMAL);
    header = (struct ethernet_header *) data;
    payload = (void *) ((uintptr_t) data + sizeof(struct ethernet_header));

    // set the packet header
    header->type = protocol_to_ethtype(protocol);
    memcpy(&header->src, device->hwaddr, device->hwaddr_len);
    tcpip_copy_from_mbuf(payload, m, data_len);

    tcpip_arp_resolve_and_send(device, addr, &header->dest, data, data_len);
}
