#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "socket.h"
#include "ip.h"
#include "udp.h"


/** handles tcpip.sendto */
void tcpip_tcpip_sendto(channel_t __ch, ident_t socket, tcpip_protocol_t network,
                        void * address, size_t address_size, uint16_t port,
                        void * payload, size_t payload_size) {

    struct addr addr;
    struct socket *sock;
    result_t r;

    sock = tcpip_get_socket_by_id(socket);

    tcpip_parse_ip_addr(&addr, network, (void *) address, address_size);
    addr.port      = port;
    addr.protocol  = network | sock->protocol;

    r = tcpip_send_udp(sock, tcpip_pack_mbuf(payload, payload_size), 0, &addr);
    send_tcpip_sendto_reply(__ch, r);
}
