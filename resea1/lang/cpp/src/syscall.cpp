#include "cpp.h"
#include <resea.h>


#ifdef KERNEL

extern "C" result_t kernel_syscall(int type, uintmax_t r1, uintmax_t r2,
                        uintmax_t r3, uintmax_t r4, uintmax_t r5, uintmax_t *ret);
#define do_syscall kernel_syscall

#else

result_t do_syscall(int type, uintmax_t r1, uintmax_t r2,
                    uintmax_t r3, uintmax_t r4, uintmax_t r5, uintmax_t *ret) {

    WARN("do_syscall in userland is not implemented yet");
    return E_NOTSUPPORTED;
}

#endif


channel_t sys_open(void) {
    uintmax_t r;

    if (do_syscall(SYSCALL_OPEN, 0, 0, 0, 0, 0, &r) != OK) {
        // TODO: implement something like errno
        return 0;
    }

    return (channel_t) r;
}


result_t sys_close(channel_t ch) {
    uintmax_t r;
    return do_syscall(SYSCALL_CLOSE, ch, 0, 0, 0, 0, &r);
}


result_t sys_wait(channel_t ch) {
    uintmax_t r;
    return do_syscall(SYSCALL_WAIT, ch, 0, 0, 0, 0, &r);
}


result_t sys_setoptions(channel_t ch, handler_t handler, void *buffer,
                        size_t size) {
    uintmax_t r;
    return do_syscall(SYSCALL_SETOPTIONS, ch, (uintmax_t) handler,
            (uintmax_t) buffer, (uintmax_t) size, 0, &r);
}


result_t sys_send(channel_t ch, payload_t *m, size_t size) {
    uintmax_t r;
    return do_syscall(SYSCALL_SEND, ch, (uintmax_t) m, (uintmax_t) size, 0, 0, &r);
}


payload_t *sys_recv(channel_t ch) {
    uintmax_t r;

    if (do_syscall(SYSCALL_RECV, ch, 0, 0, 0, 0, &r) != OK) {
        // TODO: implement something like errno
        return 0;
    }

    return (payload_t *) r;
}


result_t sys_call(channel_t ch, payload_t *m, size_t size, void *buffer, size_t buffer_size) {
    uintmax_t r;
    return do_syscall(SYSCALL_CALL, ch, (uintmax_t) m, (uintmax_t) size,
                      (uintmax_t) buffer, (uintmax_t) buffer_size, &r);
}


result_t sys_link(channel_t ch1, channel_t ch2) {
    uintmax_t r;
    return do_syscall(SYSCALL_LINK, ch1, ch2, 0, 0, 0, &r);
}


result_t sys_transfer(channel_t ch1, channel_t ch2) {
    uintmax_t r;
    return do_syscall(SYSCALL_TRANSFER, ch1, ch2, 0, 0, 0, &r);
}
