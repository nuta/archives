#include <resea.h>
#include <resea/channel.h>


channel_t create_channel() {

    return sys_open();
}


void close_channel(channel_t channel) {

    sys_close(channel);
}


void set_channel_handler(channel_t channel, handler_t handler) {

    sys_setoptions(channel, handler, nullptr, 0);
}


/**
 *  Connects to a thread group local channel
 *
 *  @param[in] id  The server's channel.
 *  @return A client side channel connected to the server.
 *
 */
channel_t connect_to_local(channel_t id) {
    channel_t client, server;

    client = create_channel();  // client (our) side
    server = create_channel();  // server side
    sys_link(client, server);
    sys_transfer(server, id);

    return client;
}


/**
 *  Serve channel forever
 *
 *  @param[in] ch       The channel.
 *  @param[in] handler  The handler_t.
 *
 */
NORETURN void serve_channel(channel_t ch, handler_t handler) {

    set_channel_handler(ch, handler);
    for(;;) {
        // TODO: receive a message, call handler, and free readonly payloads
    }
}

