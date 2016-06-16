#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>

namespace tcpip {
void server_handler(channel_t __ch, payload_t *m);

namespace tcpip_server {
void handle_open(
    channel_t __ch
    , tcpip_protocol_t transport
    , channel_t handler
);
} // namespace tcpip
namespace tcpip_server {
void handle_close(
    channel_t __ch
    , ident_t socket
);
} // namespace tcpip
namespace tcpip_server {
void handle_bind(
    channel_t __ch
    , ident_t socket
    , tcpip_protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
);
} // namespace tcpip
namespace tcpip_server {
void handle_sendto(
    channel_t __ch
    , ident_t socket
    , tcpip_protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
    , void * payload
    , size_t payload_size
);
} // namespace tcpip

} // namespace tcpip
