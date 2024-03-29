#include "tcpip.h"
#include "arp.h"
#include "device.h"
#include "ethernet.h"
#include "receive.h"
#include <string.h>
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/tcpip.h>
#include <resea/net_device.h>

using namespace tcpip;

namespace tcpip {

void ethernet_receive(struct net_device *device, void *payload, size_t length) {
    struct ethernet_header *header;
    net_type_t protocol;

    header = (struct ethernet_header *) payload;
    protocol = header->type;
    receive_packet(protocol, (void *) ((uintptr_t) payload + sizeof(*header)),
                   length - sizeof(*header));
}


void ethernet_transmit(struct net_device *device, struct addr* addr,
                             net_type_t protocol, struct mbuf *m) {

    size_t data_len;
    void *data, *payload;
    struct ethernet_header *header;

    data_len = m->total_length;
    data = allocate_memory(data_len + sizeof(struct ethernet_header),
                           resea::interfaces::memory::ALLOC_NORMAL);
    header = (struct ethernet_header *) data;
    payload = (void *) ((uintptr_t) data + sizeof(struct ethernet_header));

    // set the packet header
    header->type = protocol;
    memcpy(&header->src, device->hwaddr, device->hwaddr_len);
    copy_from_mbuf(payload, m, data_len);

    arp_resolve_and_send(device, addr, &header->dest, data, data_len);
}

} // namespace tcpip
