#include "virtio.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "virtio.h"
#include "virtio_net.h"
#include "virtio_blk.h"


handler_t virtio_handler;
channel_t virtio_server;

void virtio_startup(void) {

    virtio_server = create_channel();
    virtio_blk_init();
    virtio_net_init();
    serve_channel(virtio_server, &virtio_handler);
}
