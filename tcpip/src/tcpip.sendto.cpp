#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "socket.h"
#include "ip.h"
#include "udp.h"

using namespace tcpip;
namespace tcpip {
namespace tcpip_server {

/** handles tcpip.sendto */
void handle_sendto(channel_t __ch, ident_t socket, tcpip_protocol_t network,
            void * address, size_t address_size, uint16_t port,
            void * payload, size_t payload_size) {

    struct addr addr;
    struct socket *sock;
    result_t r;

    sock = get_socket_by_id(socket);

    parse_ip_addr(&addr, network, (void *) address, address_size);
    addr.port      = port;
    addr.protocol  = network | sock->protocol;

    r = send_udp(sock, pack_mbuf(payload, payload_size), 0, &addr);
    send_tcpip_sendto_reply(__ch, r);
}

} // namespace tcpip_server
} // namespace tcpip
