#include "virtio.h"
#include <resea.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include <resea/virtio.h>

namespace virtio {
namespace net_device_server {

void handle_listen(
    channel_t __ch
    , channel_t channel
);
void handle_transmit(
    channel_t __ch
    , void * data
    , size_t data_size
);
void handle_get_info(
    channel_t __ch
);

} // namespace net_device
} // namespace virtio
