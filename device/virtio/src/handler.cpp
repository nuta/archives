#include "virtio.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include <resea/virtio.h>
#include "storage_device_server.h"
#include "net_device_server.h"

namespace virtio {

void server_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(storage_device, read):
        DEBUG("received storage_device.read");
        storage_device_server::handle_read(
            __ch
            , (offset_t) EXTRACT(m, storage_device, read, offset)
            , (size_t) EXTRACT(m, storage_device, read, size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(storage_device, write):
        DEBUG("received storage_device.write");
        storage_device_server::handle_write(
            __ch
            , (offset_t) EXTRACT(m, storage_device, write, offset)
            , (void *) EXTRACT(m, storage_device, write, data)
            , (size_t) EXTRACT(m, storage_device, write, data_size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
        release_memory((void * ) m[__PINDEX(m, storage_device, write, data)]);
        release_memory((void * ) m[__PINDEX(m, storage_device, write, data_size)]);
#endif

    case MSGID(net_device, listen):
        DEBUG("received net_device.listen");
        net_device_server::handle_listen(
            __ch
            , (channel_t) EXTRACT(m, net_device, listen, channel)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(net_device, transmit):
        DEBUG("received net_device.transmit");
        net_device_server::handle_transmit(
            __ch
            , (void *) EXTRACT(m, net_device, transmit, data)
            , (size_t) EXTRACT(m, net_device, transmit, data_size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
        release_memory((void * ) m[__PINDEX(m, net_device, transmit, data)]);
        release_memory((void * ) m[__PINDEX(m, net_device, transmit, data_size)]);
#endif

    case MSGID(net_device, get_info):
        DEBUG("received net_device.get_info");
        net_device_server::handle_get_info(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}

} // namespace virtio