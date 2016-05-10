#include <string.h>
#include "arp.h"
#include "endian.h"
#include "printf.h"


void tcpip_receive_arp(const void *payload, size_t size) {
    struct tcpip_arp_header *header = (struct tcpip_arp_header *) payload;
    uint8_t sender_hw_addr[6], target_hw_addr[6];
    uint16_t proto_type, op;
    uint32_t sender_proto_addr, target_proto_addr;

    if (size < sizeof(struct tcpip_arp_header)) {
	WARN("too short ARP packet (size=%zu)", size);
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

    DEBUG("%s " TCPIP_FMT_MACADDR " (" TCPIP_FMT_IPV4ADDR ") -> "
      TCPIP_FMT_MACADDR " (" TCPIP_FMT_IPV4ADDR ")",
        (op == TCPIP_ARP_REQUEST)? "REQUEST" : "REPLY",
	TCPIP_FMTARG_MACADDR(sender_hw_addr),
	TCPIP_FMTARG_IPV4ADDR(sender_proto_addr),
	TCPIP_FMTARG_MACADDR(target_hw_addr),
	TCPIP_FMTARG_IPV4ADDR(target_proto_addr)
    );
}

