#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "ip.h"
#include "socket.h"

using namespace tcpip;
namespace tcpip {
namespace tcpip_server {

/** handles tcpip.bind */
void handle_bind(channel_t __ch, ident_t socket, resea::interfaces::tcpip::protocol_t network, void * address, size_t address_size, uint16_t port) {
    struct addr addr;
    struct socket *sock;

    sock = get_socket_by_id(socket);

    parse_ip_addr(&addr, network, (void *) address, address_size);
    addr.port = port;
    addr.protocol = network | sock->protocol;

    resea::interfaces::tcpip::send_bind_reply(__ch, bind_socket(sock, &addr));
}

} // namespace tcpip_server
} // namespace tcpip

#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>

namespace tcpip {
namespace tcpip_server {

/** handles tcpip.close */
void handle_close(channel_t __ch, ident_t socket) {

}

} // namespace tcpip_server
} // namespace tcpip
#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "socket.h"

using namespace tcpip;
namespace tcpip {
namespace tcpip_server {

/** handles tcpip.open */
void handle_open(channel_t __ch, resea::interfaces::tcpip::protocol_t transport, channel_t handler) {
    ident_t id;
    struct socket *sock;

    id = create_socket();
    sock = get_socket_by_id(id);
    sock->protocol = transport;
    sock->handler  = handler;

    resea::interfaces::tcpip::send_open_reply(__ch, OK, id);
}

} // namespace tcpip_server
} // namespace tcpip
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
void handle_sendto(channel_t __ch, ident_t socket, resea::interfaces::tcpip::protocol_t network,
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
    resea::interfaces::tcpip::send_sendto_reply(__ch, r);
}

} // namespace tcpip_server
} // namespace tcpip
