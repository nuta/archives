#include <assert.h>
#include "ip.h"
#include "ipv4.h"
#include "mbuf.h"
#include "endian.h"


// TODO
void tcpip_parse_ip_addr(struct addr *addr, tcpip_protocol_t protocol,
                         void *address, size_t address_size) {

    // NOTE: address is *not* null-terminated

    addr->ipv4_addr = IPV4_ADDR_LOOPBACK;
}

result_t tcpip_send_ip(struct socket *socket,
                       struct mbuf *mbuf,
                       uint8_t proto,
                       int flags, struct addr *addr) {

    return tcpip_send_ipv4(socket, mbuf, proto, flags, addr);
}
