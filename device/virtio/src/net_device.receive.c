#include <resea.h>
#include <resea/virtio.h>
#include <resea/net_device.h>
#include "virtio_net.h"


/** handles net_device.receive */
void virtio_net_device_receive(channel_t __ch) {
    result_t result;
    void* data;
    size_t size;

    result = virtio_net_receive(&data, &size);
    send_net_device_receive_reply(__ch, result, MOVE(data), size);
}
