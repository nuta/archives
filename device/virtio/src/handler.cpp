#include "virtio.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/virtio.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>
#include "handler.h"


void virtio_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(storage_device, read):
        DEBUG("received storage_device.read");
        virtio_storage_device_read(
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
        virtio_storage_device_write(
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
        virtio_net_device_listen(
            __ch
            , (channel_t) EXTRACT(m, net_device, listen, channel)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(net_device, transmit):
        DEBUG("received net_device.transmit");
        virtio_net_device_transmit(
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
        virtio_net_device_get_info(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}
