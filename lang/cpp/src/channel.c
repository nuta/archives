#include <resea.h>

#ifndef KERNEL
/**
 *  Creates a new channel to a server
 *
 *  @param[in]  interface  The interface of a server to be connected.
 *  @param[out] ch         The channel to be set up.
 *
 *  @note  In a same executable of core package, this symbol will be overwritten
 *         by core's implementation. Namely, it calls core directly.
 *
 */
result_t connect_channel(channel_t ch, interface_t interface) {

    WARN("connect_channel() is not implemented yet");
    return E_NOTSUPPORTED;
}


/**
 *  Registers a new server
 *
 *  @param[in] channel   The channel ID of the thread group to be registered.
 *  @param[in] interface  The interface of the server.
 *
 *  @note  In a same executable of core package, this symbol will be overwritten
 *         by core's implementation. Namely, it calls core directly.
 *
 */
result_t register_channel(channel_t ch, interface_t interface) {

    WARN("register_channel() is not implemented yet");
    return E_NOTSUPPORTED;
}
#endif


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

