#ifndef __TCPIP_MBUF_H__
#define __TCPIP_MBUF_H__

#include "types.h"

#define TCPIP_MBUF_END  1  // end of packet (UDP)

#define TCPIP_MBUF_SIZE        256
#define TCPIP_MBUF_HEADER_SIZE (sizeof(struct tcpip_mbuf *) + sizeof(size_t) + sizeof(uint32_t) * 2)
#define TCPIP_MBUF_DATA_SIZE   (TCPIP_MBUF_SIZE - TCPIP_MBUF_HEADER_SIZE)

struct tcpip_mqueue {
    struct tcpip_mbuf *first;
    struct tcpip_mbuf *last;
};

struct tcpip_mbuf {
    struct tcpip_mbuf *next;
    struct tcpip_addr addr;
    size_t   begin;
    size_t   length; // actual size of data
    uint32_t flags;
    uint8_t  data[TCPIP_MBUF_DATA_SIZE];
};


struct tcpip_mbuf *tcpip_allocate_mbuf();
void tcpip_free_mbuf(struct tcpip_mbuf *mbuf);
void tcpip_append_mbuf(struct tcpip_mqueue *mqueue, struct tcpip_addr *addr,
                       const void *buf, size_t size, int flags);
size_t tcpip_pop_mbuf(struct tcpip_mqueue *mqueue,
                      void *buf, size_t size, int flags,
                      struct tcpip_addr *addr);

#endif
