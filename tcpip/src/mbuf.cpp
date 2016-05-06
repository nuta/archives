#include <string.h>
#include "mbuf.h"
#include "malloc.h"
#include "printf.h"


struct tcpip_mbuf *tcpip_allocate_mbuf(struct tcpip_instance *instance) {
    struct tcpip_mbuf *mbuf;

    mbuf = (struct tcpip_mbuf *) tcpip_malloc(instance, TCPIP_MBUF_SIZE);
    mbuf->next   = nullptr;
    mbuf->begin  = 0;
    mbuf->length = 0;
    mbuf->flags  = 0;
    return mbuf;
}


void tcpip_free_mbuf(struct tcpip_instance *instance, struct tcpip_mbuf *mbuf) {

     tcpip_free(instance, mbuf);
}


void tcpip_append_mbuf(struct tcpip_mqueue *mqueue, struct tcpip_addr *addr,
                       const void *buf, size_t size, int flags) {
    uint8_t *p = (uint8_t *) buf;

    DEBUG("appending remote_port=%d, buf=%p, size=%zu", addr->port, buf, size);

    while (size > 0) {
        // TODO: lock
        struct tcpip_mbuf *new_last = tcpip_allocate_mbuf(mqueue->instance);
        size_t copy_size = ((size >= TCPIP_MBUF_DATA_SIZE)? TCPIP_MBUF_DATA_SIZE : size);

        new_last->flags  = flags;
        new_last->length = copy_size;
        memcpy(&new_last->addr, addr, sizeof(*addr));
        memcpy(&new_last->data, p, copy_size);

        if (mqueue->last) {
            mqueue->last->next = new_last;
        } else {
            mqueue->last = new_last;
        }

        if (!mqueue->first) {
            mqueue->first = new_last;
        }

        p    += copy_size;
        size -= copy_size;
    }
}


size_t tcpip_pop_mbuf(struct tcpip_mqueue *mqueue,
                      void *buf, size_t size, int flags,
                      struct tcpip_addr *addr) {
    uint8_t *p = (uint8_t *) buf;
    struct tcpip_mbuf *m = mqueue->first, *next;
    size_t num = 0;

    // TODO
    memcpy(addr, &m->addr, sizeof(*addr));

    while (m && size > 0) {
        size_t copy_size = ((size >= m->length)? m->length : size);

        memcpy(p, &m->data[m->begin], copy_size);
        num  += copy_size;

        if (copy_size < m->length) {
            m->begin  += copy_size;
            m->length -= copy_size;
            break;
        }

        next = m->next;
        tcpip_free_mbuf(mqueue->instance, m);

        if (m->flags & TCPIP_MBUF_END)
            break;

        p    += copy_size;
        size -= copy_size;
        m = next;
    }

    return num;
}

