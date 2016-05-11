#ifndef __TCPIP_MBUF_H__
#define __TCPIP_MBUF_H__

#include "types.h"

#define MBUF_SIZE        512
#define MBUF_HEADER_SIZE (sizeof(struct mbuf *) + sizeof(size_t) + sizeof(uint32_t) * 2)
#define MBUF_DATA_SIZE   (MBUF_SIZE - MBUF_HEADER_SIZE)

struct tcpip_mqueue {
    struct mbuf *first;
    struct mbuf *last;
};

struct mbuf {
    struct mbuf *next;
    struct mbuf *next_packet;
    struct addr addr;
    size_t   begin;
    size_t   length; // actual size of data
    size_t   total_length;
    uint32_t flags;
    uint8_t  data[MBUF_DATA_SIZE];
};


struct mbuf *tcpip_append_mbuf(struct mbuf *head,
                                     struct mbuf *tail,
                                     bool is_packet);
struct mbuf *tcpip_pack_mbuf(const void *buf, size_t size);
struct mbuf *tcpip_allocate_mbuf();
void tcpip_free_mbuf(struct mbuf *mbuf);
void tcpip_append_mbuf(struct tcpip_mqueue *mqueue, struct addr *addr,
                       const void *buf, size_t size, int flags);
size_t tcpip_pop_mbuf(struct tcpip_mqueue *mqueue,
                      void *buf, size_t size, int flags,
                      struct addr *addr);
result_t tcpip_copy_from_mbuf(void *buf, struct mbuf *mbuf, size_t size);

#endif
