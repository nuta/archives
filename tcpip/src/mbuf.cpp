#include <string.h>
#include "mbuf.h"
#include "malloc.h"
#include "printf.h"


result_t tcpip_copy_from_mbuf(void *buf, struct mbuf *mbuf, size_t size) {
    uint8_t *p = (uint8_t *) buf;
    size_t copy_size;

    while (mbuf) {
        copy_size = (mbuf->length - mbuf->begin < size) ?
                    mbuf->length - mbuf->begin : size;
        memcpy(p, &mbuf->data[mbuf->begin], copy_size);
        p    += copy_size;
        size -= copy_size;

        if (size > 0) {
            struct mbuf *next = mbuf->next;
            tcpip_free_mbuf(mbuf);
            mbuf = next;
        } else {
            mbuf->begin += copy_size;
            return OK;
        }
    }

    return E_NOSPACE;
}


struct mbuf *tcpip_append_mbuf(struct mbuf *head,
                               struct mbuf *tail,
                               bool is_packet) {

    if (!head)
        return tail;

    if (is_packet) {
        head->next_packet = tail;
    } else {
        head->next = tail;
        head->total_length = head->total_length + tail->total_length;
    }

    return head;
}


struct mbuf *tcpip_pack_mbuf(const void *buf, size_t size) {
    uint8_t *p;
    struct mbuf *first, *next, *m;

    p = (uint8_t *) buf;

    first = tcpip_allocate_mbuf();
    if (size > 0) {
        m = tcpip_allocate_mbuf();
        first->next = m;

        while (size > 0) {
            size_t copy_size = ((size >= MBUF_DATA_SIZE)? MBUF_DATA_SIZE : size);

            memcpy(&m->data, p, copy_size);
            size -= copy_size;
            m->begin  = 0;
            m->length = copy_size;
            first->total_length += copy_size;

            if (size > 0) {
                next = tcpip_allocate_mbuf();
                m->next = next;
                m = next;
            } else {
                m->next = nullptr;
                break;
            }
        }
    }

    return first;
}

struct mbuf *tcpip_allocate_mbuf(void) {
    struct mbuf *mbuf;

    mbuf = (struct mbuf *) tcpip_malloc(MBUF_SIZE);
    mbuf->next        = nullptr;
    mbuf->next_packet = nullptr;
    mbuf->begin  = 0;
    mbuf->length = 0;
    mbuf->flags  = 0;
    return mbuf;
}


void tcpip_free_mbuf(struct mbuf *mbuf) {

     tcpip_free(mbuf);
}
