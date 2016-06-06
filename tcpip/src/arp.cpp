#include "tcpip.h"
#include "types.h"
#include "arp.h"
#include "endian.h"
#include "printf.h"
#include <assert.h>
#include <string.h>
#include <resea/net_device.h>
#include <resea/cpp/memory.h>

static struct arp_entry *arp_table = nullptr;
static mutex_t arp_table_lock = MUTEX_UNLOCKED;
static size_t arp_entry_max = 0;


static void send_arp_request(struct net_device *device, uint32_t ipaddr) {
    struct mbuf *mbuf;
    struct tcpip_arp_header *header;

    assert(MBUF_DATA_SIZE >= sizeof(struct tcpip_arp_header));

    mbuf = tcpip_allocate_mbuf();
    mbuf->length = sizeof(struct tcpip_arp_header);
    header = (struct tcpip_arp_header *) &mbuf->data;

    header->hw_type           = tcpip_to_net_endian16(0x0001); // Ethernet
    header->proto_type        = tcpip_to_net_endian16(0x0800); // IPv4
    header->hw_len            = tcpip_to_net_endian16(6);
    header->proto_len         = tcpip_to_net_endian16(4);
    header->op                = tcpip_to_net_endian16(TCPIP_ARP_REQUEST);
    header->sender_proto_addr = tcpip_to_net_endian32(device->addr.ipv4_addr);
    header->target_proto_addr = tcpip_to_net_endian32(ipaddr);
    memcpy(&header->sender_hw_addr, &device->hwaddr, sizeof(header->sender_hw_addr));

    device->transmit(device, nullptr, NET_TYPE_ARP, mbuf);
}


static void add_pending_packet(uint32_t ipaddr,
                               struct net_device *device,
                               void *packet,
                               size_t length,
                               void *hwaddr) {

    struct arp_pending *pending = (struct arp_pending *)
        allocate_memory(sizeof(*pending), MEMORY_ALLOC_NORMAL);

    pending->packet = packet;
    pending->length = length;
    pending->hwaddr = hwaddr;
    pending->ipaddr = ipaddr;

    lock_mutex(&arp_table_lock);

    for (size_t i=0; i < arp_entry_max; i++) {
        if (arp_table[i].ipaddr == 0) {
            arp_table[i].ipaddr = ipaddr;

            // add to the pending queue
            if (!arp_table[i].pendings) {
                arp_table[i].pendings = pending;
            } else {
                struct arp_pending *prev;

                prev = arp_table[i].pendings;
                while (prev->next) {
                     prev = prev->next;
                }

                prev->next = pending;
            }

            unlock_mutex(&arp_table_lock);
            return;
        }
    }

    WARN("failed to add a pending packet to the arp table");
    unlock_mutex(&arp_table_lock);
}


// Looks up for `ipaddr` in the arp cache table. If exists,
// it fills `hwaddr` and returns true. If not, it returns false.
//
// Note that it does not block; namely, it does not send any ARP
// request.
static bool lookup(uint32_t ipaddr, void *hwaddr) {

    lock_mutex(&arp_table_lock);

    for (size_t i=0; i < arp_entry_max; i++) {
        if (arp_table[i].ipaddr == ipaddr) {
            // found
            memcpy(hwaddr, arp_table[i].hwaddr, 6);

            unlock_mutex(&arp_table_lock);
            return true;
        }
    }

    unlock_mutex(&arp_table_lock);
    return false;
}


// Resolves, set the hardware address associated to `addr`, and
// send the packet.
void tcpip_arp_resolve_and_send(struct net_device *device,
                                struct addr *addr,
                                void *hwaddr,
                                void *packet,
                                size_t packet_length) {

    if (!addr) {
        // broadcast
        uint8_t broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        result_t r;
        memcpy(hwaddr, &broadcast, sizeof(broadcast));
        call_net_device_transmit(device->ch, packet, packet_length, &r);
    } else if (lookup(addr->ipv4_addr, hwaddr)) {
        result_t r;
        call_net_device_transmit(device->ch, packet, packet_length, &r);
    } else {
        // needs ARP hardware address resolution
        DEBUG("sending ARP request");
        send_arp_request(device, addr->ipv4_addr);
        add_pending_packet(addr->ipv4_addr, device, packet, packet_length, hwaddr);
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

    // add to the ARP cache table
    lock_mutex(&arp_table_lock);

    struct arp_pending *pending;
    for (size_t i=0; i < arp_entry_max; i++) {
        if (arp_table[i].ipaddr == sender_proto_addr) {
            pending = arp_table[i].pendings;
            memcpy(&arp_table[i].hwaddr, sender_hw_addr, 6);
            arp_table[i].pendings = nullptr;
            break;
        }
    }

    unlock_mutex(&arp_table_lock);

    // send pending packets
    while (pending) {
        result_t r;
        memcpy(pending->hwaddr, sender_hw_addr, 6);

        call_net_device_transmit(pending->device->ch, pending->packet,
            pending->length, &r);

        release_memory(pending);
        pending = pending->next;
    }
}


void tcpip_init_arp() {

    arp_entry_max = 8;
    arp_table = (struct arp_entry *) allocate_memory(
                    sizeof(struct arp_entry) * arp_entry_max,
                    MEMORY_ALLOC_ZEROED);
}
