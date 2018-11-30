#include "virtio.h"
#include <resea.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include <resea/virtio.h>

namespace virtio {
namespace storage_device_server {

void handle_read(
    channel_t __ch
    , offset_t offset
    , size_t size
);
void handle_write(
    channel_t __ch
    , offset_t offset
    , void * data
    , size_t data_size
);

} // namespace storage_device
} // namespace virtio
