#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "socket.h"

using namespace tcpip;

/** handles tcpip.open */
void tcpip_tcpip_open(channel_t __ch, tcpip_protocol_t transport, channel_t handler) {
    ident_t id;
    struct socket *sock;

    id = create_socket();
    sock = get_socket_by_id(id);
    sock->protocol = transport;
    sock->handler  = handler;

    send_tcpip_open_reply(__ch, OK, id);
}
