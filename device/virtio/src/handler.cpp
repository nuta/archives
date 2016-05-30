#include "virtio.h"
#include <resea.h>
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
    case MSGID(storage_device, write):
        DEBUG("received storage_device.write");
        virtio_storage_device_write(
            __ch
            , (offset_t) EXTRACT(m, storage_device, write, offset)
            , (void *) EXTRACT(m, storage_device, write, data)
            , (size_t) EXTRACT(m, storage_device, write, data_size)
        );
        return;
    case MSGID(net_device, listen):
        DEBUG("received net_device.listen");
        virtio_net_device_listen(
            __ch
            , (channel_t) EXTRACT(m, net_device, listen, channel)
        );
        return;
    case MSGID(net_device, transmit):
        DEBUG("received net_device.transmit");
        virtio_net_device_transmit(
            __ch
            , (void *) EXTRACT(m, net_device, transmit, dest_hwaddr)
            , (size_t) EXTRACT(m, net_device, transmit, dest_hwaddr_size)
            , (void *) EXTRACT(m, net_device, transmit, src_hwaddr)
            , (size_t) EXTRACT(m, net_device, transmit, src_hwaddr_size)
            , (net_device_packet_type_t) EXTRACT(m, net_device, transmit, type)
            , (void *) EXTRACT(m, net_device, transmit, packet)
            , (size_t) EXTRACT(m, net_device, transmit, packet_size)
        );
        return;
    case MSGID(net_device, get_info):
        DEBUG("received net_device.get_info");
        virtio_net_device_get_info(
            __ch
        );
        return;
    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}
