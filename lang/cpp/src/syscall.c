#include <resea.h>

// TODO
#ifndef KERNEL


result_t sys_open(channel_t *ch) {
    return OK;
}


result_t sys_close(channel_t ch) {
    return OK;
}


result_t sys_wait(channel_t ch) {
    return OK;
}


result_t sys_setoptions(channel_t id, handler_t handler, void *buffer, size_t size) {
    return OK;
}


result_t sys_send(channel_t ch, payload_t *m) {
    return OK;
}


result_t sys_recv(channel_t channel, payload_t **m) {
    return OK;
}


result_t sys_call(channel_t ch, payload_t *m, void *buffer, size_t buffer_size) {
    return OK;
}

#endif
