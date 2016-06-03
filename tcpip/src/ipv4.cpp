#include "tcpip.h"
#include <resea.h>
#include <assert.h>
#include "ipv4.h"
#include "endian.h"
#include "icmp.h"
#include "udp.h"
#include "printf.h"


static uint16_t compute_checksum(struct mbuf *mbuf) {

    return 0; // XXX
}

result_t tcpip_send_ipv4(struct socket *socket,
                         struct mbuf *mbuf,
                         uint8_t proto,
                         int flags, struct addr *addr) {

    struct mbuf *header_mbuf;
    struct tcpip_ipv4_header *header;

    assert(MBUF_DATA_SIZE >= sizeof(struct tcpip_ipv4_header));

    header_mbuf = tcpip_allocate_mbuf();
    header_mbuf->length = sizeof(struct tcpip_ipv4_header);

    header = (struct tcpip_ipv4_header *) &header_mbuf->data;
    header->version     = 0x45;
    header->tos         = 0x00;
    header->length      = tcpip_to_net_endian16(mbuf->total_length);
    header->id          = 0;
    header->frag_offset = 0;
    header->ttl         = 64;
    header->proto       = proto;
    header->checksum    = tcpip_to_net_endian16(compute_checksum(mbuf));
    header->src_addr    = tcpip_to_net_endian32(socket->local_addr.ipv4_addr);
    header->dest_addr   = tcpip_to_net_endian32(addr->ipv4_addr);

    tcpip_append_mbuf(header_mbuf, mbuf, false);

    // XXX: for test
    tcpip_receive_ipv4(header_mbuf);
}


void tcpip_receive_ipv4(struct mbuf *mbuf) {

    struct tcpip_ipv4_header header;
    uint8_t version, proto;
    uint16_t length;
    uint32_t src_addr, dest_addr;
    size_t header_size;

    if (tcpip_copy_from_mbuf(&header, mbuf, sizeof(header)) != OK) {
        DEBUG("too short packet");
        return;
    }

    version   = header.version >> 4;
    proto     = header.proto;
    length    = tcpip_to_host_endian16(header.length);
    src_addr  = tcpip_to_host_endian32(header.src_addr);
    dest_addr = tcpip_to_host_endian32(header.dest_addr);
    header_size = sizeof(struct tcpip_ipv4_header); // TODO: care about IHL

    if (version != 4) {
        WARN("version is not 4 (version=%d)", version);
        return;
    }

    DEBUG("received IPv4: " FMT_IPV4ADDR " -> " FMT_IPV4ADDR,
      FMTARG_IPV4ADDR(src_addr),
      FMTARG_IPV4ADDR(dest_addr));

    //
    //  forward to the transport layer
    //
    struct addr src, dest;

    src.protocol   = TCPIP_PROTOCOL_IPV4;
    src.ipv4_addr  = src_addr;
    dest.protocol  = TCPIP_PROTOCOL_IPV4;
    dest.ipv4_addr = dest_addr;

    switch (proto) {
    case IPTYPE_ICMP:
        tcpip_receive_icmp(&src, &dest, mbuf);
        break;
    case IPTYPE_UDP:
        tcpip_receive_udp(&src, &dest, mbuf);
        break;
    default: WARN("unknown proto type (proto=%04x)", proto);
    }
}
