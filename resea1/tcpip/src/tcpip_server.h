#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>

namespace tcpip {
namespace tcpip_server {

void handle_open(
    channel_t __ch
    , resea::interfaces::tcpip::protocol_t transport
    , channel_t handler
);
void handle_close(
    channel_t __ch
    , ident_t socket
);
void handle_bind(
    channel_t __ch
    , ident_t socket
    , resea::interfaces::tcpip::protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
);
void handle_sendto(
    channel_t __ch
    , ident_t socket
    , resea::interfaces::tcpip::protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
    , void * payload
    , size_t payload_size
);

} // namespace tcpip
} // namespace tcpip
