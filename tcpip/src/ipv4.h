#ifndef __TCPIP_PROTOCOL_IPV4_H__
#define __TCPIP_PROTOCOL_IPV4_H__

#include "types.h"

struct tcpip_ipv4_header {
    uint8_t   version;
    uint8_t   tos;
    uint16_t  length;
    uint16_t  id;
    uint16_t  frag_offset;
    uint8_t   ttl;
    uint8_t   proto;
    uint16_t  checksum;
    uint32_t  src_addr;
    uint32_t  dest_addr;
} PACKED;

#define IPTYPE_ICMP  0x01
#define IPTYPE_UDP   0x11

// a.b.c.d
#define IPV4_ADDR(a, b, c, d)  ((a) << 24 | (b) << 16 | (c) << 8 | (d))

#define IPV4_ADDR_ANY  0
#define IPV4_ADDR_LOOPBACK  IPV4_ADDR(127, 0, 0, 1)

void tcpip_receive_ipv4(struct mbuf *mbuf);
result_t tcpip_send_ipv4(struct socket *socket,
                         struct mbuf *mbuf,
                         uint8_t proto,
                         int flags, struct addr *addr);

#endif

