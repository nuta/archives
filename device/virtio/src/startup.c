#include <resea.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "virtio.h"
#include "virtio_net.h"
#include "virtio_blk.h"


handler_t virtio_handler;

void virtio_startup(void) {
    channel_t ch;

    sys_open(&ch);
    sys_setoptions(ch, virtio_handler, NULL, 0);

    INFO("discovering virtio-blk");
    if (virtio_blk_init()) {
        register_channel(ch, INTERFACE(storage_device));
    }


    INFO("discovering virtio-net");
    if (virtio_net_init()) {
        register_channel(ch, INTERFACE(storage_device));
    }

    serve_channel(ch, &virtio_handler);
}
