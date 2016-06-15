#ifndef __TCPIP_IP_H__
#define __TCPIP_IP_H__

#include "types.h"
#include "socket.h"
#include "mbuf.h"

namespace tcpip {

void parse_ip_addr(struct addr *addr, tcpip_protocol_t protocol,
                   void *address, size_t address_size);
result_t send_ip(struct socket *socket,
                 struct mbuf *mbuf,
                 uint8_t proto,
                 int flags, struct addr *addr);

}

#endif
