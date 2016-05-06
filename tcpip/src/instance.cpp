#include "instance.h"
#include "malloc.h"


void tcpip_init(struct tcpip_instance *instance, void *pool, size_t pool_size,
    void (*callback)(struct tcpip_event *event)) {

    instance->pool = pool;
    instance->pool_size = pool_size;

    // XXX
    instance->sessions     = (struct tcpip_session *) tcpip_malloc(instance, sizeof(struct tcpip_session) * 256);
    instance->sessions_max = 256;
    instance->mbuf         = (struct tcpip_mbuf *) tcpip_malloc(instance, sizeof(struct tcpip_mbuf) * 1024);
    instance->mbuf_max     = 1024;

    instance->callback = callback;
}
