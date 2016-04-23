#include <resea.h>
#include <resea/pci.h>
#include "handler.h"


void pci_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgtype_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGTYPE(pci, listen):
    {
        DEBUG("received pci.listen");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            payload_t a4 = payloads[6];
            pci_pci_listen(__ch, (channel_t) a0, (uint32_t) a1, (uint32_t) a2, (uint32_t) a3, (uint32_t) a4);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
