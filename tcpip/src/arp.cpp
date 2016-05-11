#include <string.h>
#include "arp.h"
#include "endian.h"
#include "printf.h"


void tcpip_receive_arp(struct mbuf *mbuf) {
    struct tcpip_arp_header *header = (struct tcpip_arp_header *) &mbuf->data[mbuf->begin];
    uint8_t sender_hw_addr[6], target_hw_addr[6];
    uint16_t proto_type, op;
    uint32_t sender_proto_addr, target_proto_addr;

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

