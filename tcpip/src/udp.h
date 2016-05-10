#ifndef __TCPIP_UDP_H__
#define __TCPIP_UDP_H__

#include "types.h"
#include "socket.h"

struct tcpip_udp_header {
    uint16_t  src_port;
    uint16_t  dest_port;
    uint16_t  length;
    uint16_t  checksum;
} TCPIP_PACKED;

#define TCPIP_IPTYPE_ICMP  0x01
#define TCPIP_IPTYPE_UDP   0x11


size_t tcpip_send_udp(struct tcpip_socket *socket,
                      const void *payload, size_t size,
                      int flags, struct tcpip_addr *addr);
void tcpip_receive_udp(struct tcpip_addr *src_addr, struct tcpip_addr *dest_addr,
  const void *payload, size_t size);

#endif
