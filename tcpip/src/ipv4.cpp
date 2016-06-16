#include "tcpip.h"
#include <resea.h>
#include <assert.h>
#include "ipv4.h"
#include "endian.h"
#include "icmp.h"
#include "udp.h"
#include "printf.h"
#include "arp.h"
#include "route.h"
#include "device.h"


using namespace tcpip;

namespace tcpip {

static uint16_t compute_checksum(struct mbuf *mbuf) {

    return 0; // XXX
}


static bool is_loopback_address(uint32_t addr) {

    return addr == IPV4_ADDR_LOOPBACK;
}


result_t send_ipv4(struct socket *socket,
                   struct mbuf *mbuf,
                   uint8_t proto,
                   int flags, struct addr *addr) {

    struct mbuf *header_mbuf;
    struct ipv4_header *header;

    assert(MBUF_DATA_SIZE >= sizeof(struct ipv4_header));

    header_mbuf = allocate_mbuf();
    header_mbuf->length = sizeof(struct ipv4_header);

    header = (struct ipv4_header *) &header_mbuf->data;
    header->version     = 0x45;
    header->tos         = 0x00;
    header->length      = to_net_endian16(mbuf->total_length);
    header->id          = 0;
    header->frag_offset = 0;
    header->ttl         = 64;
    header->proto       = proto;
    header->checksum    = to_net_endian16(compute_checksum(mbuf));
    header->src_addr    = to_net_endian32(socket->local_addr.ipv4_addr);
    header->dest_addr   = to_net_endian32(addr->ipv4_addr);

    append_mbuf(header_mbuf, mbuf, false);

    if (is_loopback_address(addr->ipv4_addr)) {
        // loopback
        receive_ipv4(header_mbuf);
    } else {
        struct net_device *device = route(addr);

        if (header_mbuf->total_length > device->max_data_size) {
            WARN("too long packet; aborting transmission");
            return E_INVALID;
        }

        device->transmit(device, addr, NET_TYPE_IPV4, header_mbuf);
    }

    return OK;
}


void receive_ipv4(struct mbuf *mbuf) {
    struct ipv4_header header;
    uint8_t version, proto;
    uint32_t src_addr, dest_addr;

    if (copy_from_mbuf(&header, mbuf, sizeof(header)) != OK) {
        DEBUG("too short packet");
        return;
    }

    version   = header.version >> 4;
    proto     = header.proto;
    src_addr  = to_host_endian32(header.src_addr);
    dest_addr = to_host_endian32(header.dest_addr);

    // TODO: check IHL

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

    src.protocol   = resea::interfaces::tcpip::PROTOCOL_IPV4;
    src.ipv4_addr  = src_addr;
    dest.protocol  = resea::interfaces::tcpip::PROTOCOL_IPV4;
    dest.ipv4_addr = dest_addr;

    switch (proto) {
    case IPTYPE_ICMP:
        receive_icmp(&src, &dest, mbuf);
        break;
    case IPTYPE_UDP:
        receive_udp(&src, &dest, mbuf);
        break;
    default: WARN("unknown proto type (proto=%04x)", proto);
    }
}

} // namespace tcpip
