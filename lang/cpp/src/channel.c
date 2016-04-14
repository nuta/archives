#include <resea.h>
#include <resea/channel.h>

/**
 *  Connects to a thread group local channel
 *
 *  @param[in] id  The server's channel.
 *  @return A client side channel connected to the server.
 *
 */
channel_t connect_to_local(channel_t id) {
    channel_t client, server;

    client = sys_open();  // client (our) side
    server = sys_open();  // server side
    sys_link(client, server);
    sys_transfer(server, id);

    return client;
}


result_t connect_channel(channel_t ch, interface_t interface) {
    result_t r;

    // we assume that the first (1) channel is listened by a channel server
    call_channel_connect(connect_to_local(1), ch, interface, &r);
    return r;
}


result_t register_channel(channel_t ch, interface_t interface) {
    result_t r;

    // we assume that the first (1) channel is listened by a channel server
    call_channel_register(connect_to_local(1), ch, interface, &r);
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

