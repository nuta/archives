#ifndef __TCPIP_UDP_H__
#define __TCPIP_UDP_H__

#include "types.h"
#include "socket.h"

namespace tcpip {

struct udp_header {
    uint16_t  src_port;
    uint16_t  dest_port;
    uint16_t  length;
    uint16_t  checksum;
} PACKED;

#define IPTYPE_ICMP  0x01
#define IPTYPE_UDP   0x11

result_t send_udp(struct socket *socket,
                  struct mbuf *mbuf,
                  int flags, struct addr *addr);
void receive_udp(struct addr *src_addr, struct addr *dest_addr, struct mbuf *mbuf);

} // namespace tcpip

#endif
