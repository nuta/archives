#ifndef __TCPIP_INSTANCE_H__
#define __TCPIP_INSTANCE_H__

#include "types.h"
#include "session.h"

struct tcpip_event {
};


struct tcpip_instance {
    void *pool;        // the pointer to the memory pool used by tcpip internally
    size_t pool_size;  // the size of the memory pool

    struct tcpip_session *sessions;
    size_t sessions_max;

    struct tcpip_mbuf *mbuf;
    size_t mbuf_max;

    void (*callback)(struct tcpip_event *event);
};


void tcpip_init(struct tcpip_instance *instance, void *pool, size_t pool_size,
    void (*callback)(struct tcpip_event *event));


#endif
