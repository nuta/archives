#include "ipv4.h"
#include "endian.h"
#include "icmp.h"
#include "udp.h"
#include "printf.h"


void tcpip_receive_ipv4(struct tcpip_instance *instance, const void *payload, size_t size) {

    struct tcpip_ipv4_header *header = (struct tcpip_ipv4_header *) payload;
    uint8_t version, proto;
    uint16_t length, checksum;
    uint32_t src_addr, dest_addr;
    size_t header_size;

    version   = header->version >> 4;
    proto     = header->proto;
    length    = tcpip_to_host_endian16(header->length);
    checksum  = tcpip_to_host_endian16(header->checksum);
    src_addr  = tcpip_to_host_endian32(header->src_addr);
    dest_addr = tcpip_to_host_endian32(header->dest_addr);
    header_size = sizeof(struct tcpip_ipv4_header); // TODO: care about IHL

    if (version != 4) {
	WARN("version is not 4 (version=%d)", version);
	return;
    }

    DEBUG(TCPIP_FMT_IPV4ADDR " -> " TCPIP_FMT_IPV4ADDR,
      TCPIP_FMTARG_IPV4ADDR(src_addr),
      TCPIP_FMTARG_IPV4ADDR(dest_addr));

    const void * ptr = (const void *) ((uintptr_t) payload + header_size);
    size_t len = length - header_size;
    struct tcpip_addr src  = { .protocol = TCPIP_IPV4, .ipv4_addr = src_addr };
    struct tcpip_addr dest = { .protocol = TCPIP_IPV4, .ipv4_addr = dest_addr };

    switch (proto) {
    case TCPIP_IPTYPE_ICMP:
	tcpip_receive_icmp(instance, &src, &dest, ptr, len);
	break;
    case TCPIP_IPTYPE_UDP:
	tcpip_receive_udp(instance, &src, &dest, ptr, len);
	break;
    default: WARN("unknown proto type (proto=%04x)", proto);
    }
}
