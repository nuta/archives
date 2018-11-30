#include "pci.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/pci.h>
#include "pci_server.h"

namespace pci {

void server_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(pci, listen):
        DEBUG("received pci.listen");
        pci_server::handle_listen(
            __ch
            , (channel_t) EXTRACT(m, pci, listen, ch)
            , (uint32_t) EXTRACT(m, pci, listen, vendor)
            , (uint32_t) EXTRACT(m, pci, listen, device)
            , (uint32_t) EXTRACT(m, pci, listen, subvendor)
            , (uint32_t) EXTRACT(m, pci, listen, subdevice)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}

} // namespace pci