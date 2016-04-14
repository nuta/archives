#include <resea.h>
#include <resea/channel.h>


channel_t connect_to_channel_server(void) {

    // we assume that the first (1) channel is listened by
    // a channel server
    channel_t client, server;

    sys_open(&client);  // client (our) side
    sys_open(&server);  // server side
    sys_link(client, server);
    sys_transfer(server, 1);

    return client;
}


result_t connect_channel(channel_t ch, interface_t interface) {
    result_t r;

    call_channel_connect(connect_to_channel_server(), ch, interface, &r);
    return r;
}


result_t register_channel(channel_t ch, interface_t interface) {
    result_t r;

    call_channel_register(connect_to_channel_server(), ch, interface, &r);
    return r;
}


/**
 *  Serve channel forever
 *
 *  @param[in] ch       The channel.
 *  @param[in] handler  The handler_t.
 *
 */
NORETURN void serve_channel(channel_t ch, handler_t handler) {

    sys_setoptions(ch, handler, NULL, 0);
    for(;;) {
        // TODO: receive a message, call handler, and free readonly payloads
    }
}

