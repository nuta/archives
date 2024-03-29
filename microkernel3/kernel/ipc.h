#ifndef __IPC_H__
#define __IPC_H__

#include <kernel/types.h>
#include "list.h"

typedef umax_t channel_t;
typedef umax_t header_t;
typedef umax_t payload_t;
typedef umax_t header_t;
typedef header_t error_t;
typedef umax_t usize_t;
typedef char * string_t;
typedef u8_t * buffer_t;

#define PAYLOAD_INLINE  0
#define PAYLOAD_OOL     1
#define PAYLOAD_CHANNEL 2
#define PAYLOAD_TYPES_OFFSET 8ULL
#define MINOR_ID_OFFSET 16ULL
#define MAJOR_ID_OFFSET 24ULL
#define PAYLOAD_TYPES(header) (((header) >> PAYLOAD_TYPES_OFFSET) & 0xff)
#define PAYLOAD_TYPE(type, n) (((type) >> (PAYLOAD_TYPES_OFFSET + (2 * (n)))) & 0x3)
#define MSGTYPE(header) (((header) >> MINOR_ID_OFFSET) & 0xffff)
#define ERRTYPE(header) ((header) & 0xff)
#define IFTYPE(header) (((header) >> MAJOR_ID_OFFSET) & 0xff)
#define METHODTYPE(header) (((header) >> MINOR_ID_OFFSET) & 0xff)
#define NOTIFICATION_MSG ((0ULL << MAJOR_ID_OFFSET) | (1ULL << MINOR_ID_OFFSET))

enum {
    /* Errors returned from the app. */
    ERROR_NONE = 0,
    ERROR_UNKNOWN_MSG = 1,
    ERROR_NOT_IMPLEMENTED = 2,
    ERROR_INVALID_ARG = 3,
    ERROR_INVALID_MSG = 4,

    /* errors returned by kernel */
    ERROR_NO_MEMORY = 200,
    ERROR_INVALID_CH = 201,
    ERROR_CH_NOT_LINKED = 202,
    ERROR_CH_NOT_TRANSFERED = 203,
    ERROR_CH_IN_USE = 204,
    ERROR_CH_CLOSED = 205,

    /* Internally used in the server. */
    ERROR_DONT_REPLY = 255,

    /* The maximum error number must be less than NULL_PAGE_SIZE. */
};

struct waitqueue {
    struct waitqueue *next;
    struct thread *thread;
};

DEFINE_LIST(waitqueue, struct waitqueue)

struct msg {
    header_t header;
    channel_t sent_from;
    payload_t payloads[4];
};

#define CHANNEL_UNUSED 0
#define CHANNEL_OPENED 1
#define CHANNEL_CLOSED 2

struct thread;
struct channel {
    int state;
    payload_t notifications;
    channel_t cid;
    struct process *process;
    struct channel *linked_to;
    struct channel *transfer_to;
    struct thread *receiver;
    struct thread *sender;
    struct waitqueue *wq;
    kmutex_t lock;
    int refs;
};

void channel_link(struct channel *ch1, struct channel *ch2);
void channel_transfer(struct channel *from, struct channel *to);
struct channel *channel_create(struct process *process);
void channel_close(struct channel *ch);
struct msg *channel_recv(struct channel *ch);
error_t channel_connect(struct channel *server, struct process *client);
struct channel *get_channel_by_id(channel_t cid);
error_t do_notify(struct process *proc, channel_t cid, payload_t and_mask, payload_t or_mask);

header_t sys_send(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3
);

struct msg *sys_recv(
    channel_t ch
);

struct msg *sys_call(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3
);

struct msg *sys_replyrecv(
    channel_t client,
    header_t type,
    payload_t r0,
    payload_t r1,
    payload_t r2,
    payload_t r3
);

channel_t sys_connect(channel_t server);
error_t sys_notify(channel_t ch, payload_t and_mask, payload_t or_mask);

static inline header_t ipc_recv(
    channel_t ch,
    channel_t *from,
    payload_t *a0,
    payload_t *a1,
    payload_t *a2,
    payload_t *a3
) {
    struct msg *msg = sys_recv(ch);
    if (IS_ERROR_PTR(msg)) {
        return ERROR_FROM_PTR(msg);
    }

    *from = msg->sent_from;
    *a0 = msg->payloads[0];
    *a1 = msg->payloads[1];
    *a2 = msg->payloads[2];
    *a3 = msg->payloads[3];
    return msg->header;
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
    struct msg *msg = sys_call(ch, type, a0, a1, a2, a3);
    if (IS_ERROR_PTR(msg)) {
        return ERROR_FROM_PTR(msg);
    }

    *r0 = msg->payloads[0];
    *r1 = msg->payloads[1];
    *r2 = msg->payloads[2];
    *r3 = msg->payloads[3];
    return msg->header;
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
    struct msg *msg = sys_replyrecv(*client, type, r0, r1, r2, r3);
    if (IS_ERROR_PTR(msg)) {
        return ERROR_FROM_PTR(msg);
    }

    *client = msg->sent_from;
    *a0 = msg->payloads[0];
    *a1 = msg->payloads[1];
    *a2 = msg->payloads[2];
    *a3 = msg->payloads[3];
    return msg->header;
}

static inline channel_t ipc_connect(channel_t server) {
    return sys_connect(server);
}

static inline error_t ipc_notify(channel_t ch, payload_t and_mask, payload_t or_mask) {
    return sys_notify(ch, and_mask, or_mask);
}

#endif
