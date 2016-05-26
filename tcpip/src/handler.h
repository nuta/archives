#include "tcpip.h"
#include <resea.h>

void tcpip_tcpip_open(
    channel_t __ch
    , tcpip_protocol_t transport
    , channel_t handler
);
void tcpip_tcpip_close(
    channel_t __ch
    , ident_t socket
);
void tcpip_tcpip_bind(
    channel_t __ch
    , ident_t socket
    , tcpip_protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
);
void tcpip_tcpip_sendto(
    channel_t __ch
    , ident_t socket
    , tcpip_protocol_t network
    , void * address
    , size_t address_size
    , uint16_t port
    , void * payload
    , size_t payload_size
);
