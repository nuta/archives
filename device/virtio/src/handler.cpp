#include <resea.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "handler.h"


void virtio_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgtype_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGTYPE(storage_device, read):
    {
        DEBUG("received storage_device.read");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            virtio_storage_device_read(__ch, (offset_t) a0, (size_t) a1);
            return;
    }
    case MSGTYPE(storage_device, write):
    {
        DEBUG("received storage_device.write");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            virtio_storage_device_write(__ch, (offset_t) a0, (void *) a1, (size_t) a2);
            return;
    }
    case MSGTYPE(net_device, receive):
    {
        DEBUG("received net_device.receive");
            virtio_net_device_receive(__ch);
            return;
    }
    case MSGTYPE(net_device, transmit):
    {
        DEBUG("received net_device.transmit");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            virtio_net_device_transmit(__ch, (void *) a0, (size_t) a1);
            return;
    }
    case MSGTYPE(net_device, get_hardware_address):
    {
        DEBUG("received net_device.get_hardware_address");
            virtio_net_device_get_hardware_address(__ch);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
