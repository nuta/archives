#include "virtio.h"
#include <resea.h>
#include <resea/storage_device.h>
#include <resea/net_device.h>
#include <resea/virtio.h>

namespace virtio {
void server_handler(channel_t __ch, payload_t *m);

namespace storage_device_server {
void handle_read(
    channel_t __ch
    , offset_t offset
    , size_t size
);
} // namespace storage_device
namespace storage_device_server {
void handle_write(
    channel_t __ch
    , offset_t offset
    , void * data
    , size_t data_size
);
} // namespace storage_device
namespace net_device_server {
void handle_listen(
    channel_t __ch
    , channel_t channel
);
} // namespace net_device
namespace net_device_server {
void handle_transmit(
    channel_t __ch
    , void * data
    , size_t data_size
);
} // namespace net_device
namespace net_device_server {
void handle_get_info(
    channel_t __ch
);
} // namespace net_device

} // namespace virtio
