#ifndef __IPC_H__
#define __IPC_H__

#include <kernel/types.h>
#include "thread.h"

typedef umax_t channel_t;
typedef umax_t header_t;
typedef umax_t payload_t;
typedef umax_t header_t;
typedef header_t error_t;
typedef umax_t usize_t;
typedef char * string_t;
typedef u8_t * buffer_t;

#define PAYLOAD_TYPE(type, n) (((type) >> (3 * (n))) & 0x3)
#define PAYLOAD_INLINE  0
#define PAYLOAD_OOL     1
#define PAYLOAD_CHANNEL 2

#define TYPES_OFFSET   0ULL
#define ERROR_OFFSET    24ULL
#define MINOR_ID_OFFSET 32ULL
#define MAJOR_ID_OFFSET 40ULL
#define MSGTYPE(header) ((header) >> MINOR_ID_OFFSET)
#define ERRTYPE(header) (((header) >> ERROR_OFFSET) & 0xff)
#define MSG_SERVICE_ID(header) (((header) >> MAJOR_ID_OFFSET) & 0xffff)
#define MSG_ID(header) (((header) >> MINOR_ID_OFFSET) & 0xff)

enum {
    /* Errors returned from the app. */
    ERROR_NONE = 0,
    ERROR_UNKNOWN_MSG = 1,

    /* errors returned by kernel */
    ERROR_NO_MEMORY = 200,
    ERROR_INVALID_CH = 201,
    ERROR_CH_NOT_LINKED = 202,
    ERROR_CH_NOT_TRANSFERED = 203,
    ERROR_CH_IN_USE = 204,

    /* Internally used in the server. */
    ERROR_DONT_REPLY = 255,
};

struct waitqueue {
    struct waitqueue *next;
    struct thread *thread;
};

DEFINE_LIST(waitqueue, struct waitqueue)

struct channel {
    int flags;
    channel_t cid;
    struct process *process;
    struct channel *linked_to;
    struct channel *transfer_to;
    struct thread *receiver;
    struct thread *sender;
    struct waitqueue *wq;
    payload_t sent_from, header, buffer[5];
};

struct channel *channel_create(struct process *process);
error_t channel_connect(struct channel *server, struct process *client);

header_t sys_send(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3
);

header_t sys_recv(
    channel_t ch,
    payload_t *rs
);

header_t sys_call(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3,
    payload_t *rs
);

header_t sys_replyrecv(
    channel_t client,
    header_t type,
    payload_t r0,
    payload_t r1,
    payload_t r2,
    payload_t r3,
    channel_t *rs
);

channel_t sys_connect(channel_t server);

static inline header_t ipc_recv(
    channel_t ch,
    channel_t *from,
    payload_t *a0,
    payload_t *a1,
    payload_t *a2,
    payload_t *a3
) {
    payload_t rs[5];
    header_t header = sys_recv(ch, (payload_t *) &rs);
    *from = rs[0];
    *a0 = rs[1];
    *a1 = rs[2];
    *a2 = rs[3];
    *a3 = rs[4];
    return header;
}

static inline header_t ipc_send(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3
) {
    return sys_send(ch, type, a0, a1, a2, a3);
}

static inline header_t ipc_call(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3,
    payload_t *r0,
    payload_t *r1,
    payload_t *r2,
    payload_t *r3
) {
    payload_t rs[5];
    header_t header = sys_call(ch, type, a0, a1, a2, a3, (payload_t *) &rs);
    *r0 = rs[1];
    *r1 = rs[2];
    *r2 = rs[3];
    *r3 = rs[4];
    return header;
}

static inline header_t ipc_replyrecv(
    channel_t *client,
    header_t type,
    payload_t r0,
    payload_t r1,
    payload_t r2,
    payload_t r3,
    payload_t *a0,
    payload_t *a1,
    payload_t *a2,
    payload_t *a3
) {
    payload_t rs[5];
    header_t header = sys_replyrecv(*client, type, r0, r1, r2, r3, (payload_t *) &rs);
    *client = rs[0];
    *a0 = rs[1];
    *a1 = rs[2];
    *a2 = rs[3];
    *a3 = rs[4];
    return header;
}

static inline channel_t ipc_connect(channel_t server) {
    return sys_connect(server);
}

#endif
