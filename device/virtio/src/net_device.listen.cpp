#include "virtio.h"
#include "virtio_net.h"
#include <resea.h>
#include <resea/virtio.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>

namespace virtio {
namespace net_device_server {

/** handles net_device.listen */
void handle_listen(
    channel_t __ch
    , channel_t channel
) {

    virtio_net_listener = channel;
    send_net_device_listen_reply(__ch, OK);
}

} // namespace net_device_server
} // namespace virtio
