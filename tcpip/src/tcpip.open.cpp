#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "socket.h"

using namespace tcpip;
namespace tcpip {
namespace tcpip_server {

/** handles tcpip.open */
void open(channel_t __ch, tcpip_protocol_t transport, channel_t handler) {
    ident_t id;
    struct socket *sock;

    id = create_socket();
    sock = get_socket_by_id(id);
    sock->protocol = transport;
    sock->handler  = handler;

    send_tcpip_open_reply(__ch, OK, id);
}

} // namespace tcpip_server
} // namespace tcpip
