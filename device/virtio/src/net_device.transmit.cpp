#include "virtio.h"
#include "virtio_net.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>

namespace virtio {
namespace net_device_server {

/** handles net_device.transmit */
void handle_transmit(
    channel_t __ch
    , void * data
    , size_t data_size
) {
    result_t result;

    result = virtio_net_transmit(data, data_size);
    resea::interfaces::net_device::send_transmit_reply(__ch, result);
}

} // namespace net_device_server
} // namespace virtio
