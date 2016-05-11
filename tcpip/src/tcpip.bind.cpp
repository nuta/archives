#include <resea.h>
#include <resea/tcpip.h>
#include "ip.h"
#include "socket.h"


/** handles tcpip.bind */
void tcpip_tcpip_bind(channel_t __ch, ident_t socket, tcpip_protocol_t network, void * address, size_t address_size, uint16_t port) {
    struct addr addr;
    struct socket *sock;

    sock = tcpip_get_socket_by_id(socket);

    tcpip_parse_ip_addr(&addr, network, (void *) address, address_size);
    addr.port = port;
    addr.protocol = network | sock->protocol;

    send_tcpip_bind_reply(__ch, tcpip_bind_socket(sock, &addr));
}
