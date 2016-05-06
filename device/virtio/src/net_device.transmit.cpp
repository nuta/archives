#include <resea.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include "virtio_net.h"


/** handles net_device.transmit */
void virtio_net_device_transmit(channel_t __ch, void * data, size_t size) {

    result_t result;

    result = virtio_net_transmit(data, size);
    send_net_device_transmit_reply(__ch, result);
}
