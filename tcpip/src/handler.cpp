#include "tcpip.h"
#include <resea.h>
#include <resea/tcpip.h>
#include "handler.h"


void tcpip_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    while(!tcpip_initialized);

    switch (EXTRACT_MSGID(m)) {
    case MSGID(tcpip, open):
        DEBUG("received tcpip.open");
        tcpip_tcpip_open(
            __ch
            , (tcpip_protocol_t) EXTRACT(m, tcpip, open, transport)
            , (channel_t) EXTRACT(m, tcpip, open, handler)
        );
        return;
    case MSGID(tcpip, close):
        DEBUG("received tcpip.close");
        tcpip_tcpip_close(
            __ch
            , (ident_t) EXTRACT(m, tcpip, close, socket)
        );
        return;
    case MSGID(tcpip, bind):
        DEBUG("received tcpip.bind");
        tcpip_tcpip_bind(
            __ch
            , (ident_t) EXTRACT(m, tcpip, bind, socket)
            , (tcpip_protocol_t) EXTRACT(m, tcpip, bind, network)
            , (void *) EXTRACT(m, tcpip, bind, address)
            , (size_t) EXTRACT(m, tcpip, bind, address_size)
            , (uint16_t) EXTRACT(m, tcpip, bind, port)
        );
        return;
    case MSGID(tcpip, sendto):
        DEBUG("received tcpip.sendto");
        tcpip_tcpip_sendto(
            __ch
            , (ident_t) EXTRACT(m, tcpip, sendto, socket)
            , (tcpip_protocol_t) EXTRACT(m, tcpip, sendto, network)
            , (void *) EXTRACT(m, tcpip, sendto, address)
            , (size_t) EXTRACT(m, tcpip, sendto, address_size)
            , (uint16_t) EXTRACT(m, tcpip, sendto, port)
            , (void *) EXTRACT(m, tcpip, sendto, payload)
            , (size_t) EXTRACT(m, tcpip, sendto, payload_size)
        );
        return;
    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}
