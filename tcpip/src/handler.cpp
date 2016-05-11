#include <resea.h>
#include <resea/tcpip.h>
#include "handler.h"


void tcpip_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgtype_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGTYPE(tcpip, open):
    {
        DEBUG("received tcpip.open");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            tcpip_tcpip_open(__ch, (tcpip_protocol_t) a0, (channel_t) a1);
            return;
    }
    case MSGTYPE(tcpip, close):
    {
        DEBUG("received tcpip.close");
            payload_t a0 = payloads[2];
            tcpip_tcpip_close(__ch, (ident_t) a0);
            return;
    }
    case MSGTYPE(tcpip, bind):
    {
        DEBUG("received tcpip.bind");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            payload_t a4 = payloads[6];
            tcpip_tcpip_bind(__ch, (ident_t) a0, (tcpip_protocol_t) a1, (void *) a2, (size_t) a3, (uint16_t) a4);
            return;
    }
    case MSGTYPE(tcpip, sendto):
    {
        DEBUG("received tcpip.sendto");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            payload_t a4 = payloads[6];
            payload_t a5 = payloads[7];
            payload_t a6 = payloads[8];
            tcpip_tcpip_sendto(__ch, (ident_t) a0, (tcpip_protocol_t) a1, (void *) a2, (size_t) a3, (uint16_t) a4, (void *) a5, (size_t) a6);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
