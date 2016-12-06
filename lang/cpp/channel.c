#include <resea.h>


channel_t connect_to_local(channel_t to) {

    // TODO: check return values

    channel_t client = open();
    channel_t proxy = open();

    link(client, proxy);
    transfer(proxy, to);
    return client;
}


channel_t transfer_to(channel_t to) {

    // TODO: check return values
    channel_t proxy = open();
    transfer(proxy, to);
    return proxy;
}
