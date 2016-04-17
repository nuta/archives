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

