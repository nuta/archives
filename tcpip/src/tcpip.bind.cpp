#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "ip.h"
#include "socket.h"

using namespace tcpip;

/** handles tcpip.bind */
void tcpip_tcpip_bind(channel_t __ch, ident_t socket, tcpip_protocol_t network, void * address, size_t address_size, uint16_t port) {
    struct addr addr;
    struct socket *sock;

    sock = get_socket_by_id(socket);

    parse_ip_addr(&addr, network, (void *) address, address_size);
    addr.port = port;
    addr.protocol = network | sock->protocol;

    send_tcpip_bind_reply(__ch, bind_socket(sock, &addr));
}
