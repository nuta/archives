#include <resea.h>
#include <resea/channel.h>
#include <resea/net_device.h>
#include <resea/storage_device.h>
#include "virtio.h"
#include "virtio_net.h"
#include "virtio_blk.h"


handler_t virtio_handler;

void virtio_startup(void) {
    channel_t ch;
    result_t r;

    ch = sys_open();
    sys_setoptions(ch, virtio_handler, NULL, 0);

    INFO("discovering virtio-blk");
    if (virtio_blk_init()) {
        call_channel_register(connect_to_local(1), ch,
	    INTERFACE(net_device), &r);
    }


    INFO("discovering virtio-net");
    if (virtio_net_init()) {
        call_channel_register(connect_to_local(1), ch,
	    INTERFACE(storage_device), &r);
    }

    serve_channel(ch, &virtio_handler);
}
