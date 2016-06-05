#include <string.h>
#include "tcpip.h"
#include "types.h"
#include "arp.h"
#include "endian.h"
#include "printf.h"
#include <resea/net_device.h>
#include <resea/cpp/memory.h>


static struct arp_entry *arp_table = nullptr;
static size_t arp_entry_max = 0;


// Looks up for `addr` in the arp cache table. If exists,
// it fills `hwaddr` and returns true. If not, it returns false.
//
// Note that it does not block; namely, it does not send any ARP
// request.
static bool lookup(struct addr *addr, void *hwaddr) {

    for (size_t i=0; i < arp_entry_max; i++) {
        if (arp_table[i].ipaddr == addr->ipv4_addr) {
            // found
            // FIXME: needs lock or something
            memcpy(hwaddr, arp_table[i].hwaddr, 6);
            return true;
        }
    }

    return false;
}


// Resolves, set the hardware address associated to `addr`, and
// send the packet.
void tcpip_arp_resolve_and_send(struct net_device *device,
                                struct addr *addr,
                                void *hwaddr,
                                void *packet,
                                size_t packet_length) {

    if (lookup(addr, hwaddr)) {
        result_t r;
        call_net_device_transmit(device->ch, packet, packet_length, &r);
    } else {
        // needs ARP hardware address resolution
        BUG("ARP resolution is not implemented");
    }
}

void tcpip_receive_arp(struct mbuf *mbuf) {
    struct tcpip_arp_header *header;
    uint8_t sender_hw_addr[6], target_hw_addr[6];
    uint16_t proto_type, op;
    uint32_t sender_proto_addr, target_proto_addr;

    header = (struct tcpip_arp_header *) &mbuf->data[mbuf->begin];

    if (mbuf->length - mbuf->begin < sizeof(struct tcpip_arp_header)) {
        WARN("too short ARP packet (size=%zu)", mbuf->length - mbuf->begin);
        return;
    }

    memcpy(sender_hw_addr, header->sender_hw_addr, sizeof(sender_hw_addr));
    memcpy(target_hw_addr, header->target_hw_addr, sizeof(target_hw_addr));
    op = tcpip_to_host_endian16(header->op);
    proto_type = tcpip_to_host_endian16(header->proto_type);
    sender_proto_addr = tcpip_to_host_endian32(header->sender_proto_addr);
    target_proto_addr = tcpip_to_host_endian32(header->target_proto_addr);

    if (proto_type != 0x0800) {
        WARN("non-ipv4 ARP packet (proto_type=%04x)", proto_type);
        return;
    }

    DEBUG("%s " FMT_MACADDR " (" FMT_IPV4ADDR ") -> "
      FMT_MACADDR " (" FMT_IPV4ADDR ")",
        (op == TCPIP_ARP_REQUEST)? "REQUEST" : "REPLY",
        FMTARG_MACADDR(sender_hw_addr),
        FMTARG_IPV4ADDR(sender_proto_addr),
        FMTARG_MACADDR(target_hw_addr),
        FMTARG_IPV4ADDR(target_proto_addr)
    );
}


void tcpip_init_arp() {

    arp_entry_max = 8;
    arp_table = (struct arp_entry *) allocate_memory(
                    sizeof(struct arp_entry) * arp_entry_max,
                    MEMORY_ALLOC_ZEROED);
}
