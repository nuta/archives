#include "virtio.h"
#include "virtio_net.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>

/** handles net_device.transmit */
void virtio_net_device_transmit(
    channel_t __ch
    , void * data
    , size_t data_size
) {
    result_t result;

    result = virtio_net_transmit(data, data_size);
    send_net_device_transmit_reply(__ch, result);
}
