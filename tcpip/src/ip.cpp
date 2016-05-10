#include <assert.h>
#include "ip.h"
#include "ipv4.h"
#include "endian.h"


int tcpip_send_ip(struct tcpip_socket *socket,
                  struct tcpip_mbuf *payload, size_t size,
                  int flags, struct tcpip_addr *addr) {

    assert(TCPIP_MBUF_DATA_SIZE >= sizeof(struct tcpip_ipv4_header));

    struct tcpip_mbuf *m = tcpip_allocate_mbuf();
    struct tcpip_ipv4_header *header = (struct tcpip_ipv4_header *) &m->data;
    uint16_t checksum; // TODO
    uint8_t proto; // TODO

    header->version     = 0x45;
    header->tos         = 0x00;
    header->length      = tcpip_to_net_endian16(size);
    header->id          = 0;
    header->frag_offset = 0;
    header->ttl         = 64;
    header->proto       = proto;
    header->checksum    = tcpip_to_net_endian16(checksum);
    header->src_addr    = tcpip_to_net_endian32(socket->local_addr.ipv4_addr);
    header->dest_addr   = tcpip_to_net_endian32(addr->ipv4_addr);

    return 0;
}
