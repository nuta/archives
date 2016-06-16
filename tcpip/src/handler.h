#include "tcpip.h"
#include <resea.h>

namespace tcpip {
void handler(channel_t __ch, payload_t *m);

namespace tcpip_server {
void open(
    channel_t __ch
    , tcpip_protocol_t transport
    , channel_t handler
);
} // namespace tcpip
namespace tcpip_server {
void close(
    channel_t __ch
    , ident_t socket
);
} // namespace tcpip
namespace tcpip_server {
void bind(
    channel_t __ch
    , ident_t socket
    , tcpip_protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
);
} // namespace tcpip
namespace tcpip_server {
void sendto(
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
