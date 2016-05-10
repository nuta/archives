#include <assert.h>
#include <string.h>
#include "udp.h"
#include "endian.h"
#include "socket.h"
#include "printf.h"


size_t tcpip_send_udp(struct tcpip_socket *socket,
                      const void *payload, size_t size,
                      int flags, struct tcpip_addr *addr) {

    assert(TCPIP_MBUF_DATA_SIZE >= sizeof(struct tcpip_udp_header));

    uint8_t *p = (uint8_t *) payload;
    struct tcpip_mbuf *first, *m, *next;

    first = tcpip_allocate_mbuf();
    struct tcpip_udp_header *header = (struct tcpip_udp_header *) &first->data;

    header->src_port  = tcpip_to_net_endian16(socket->local_addr.port);
    header->dest_port = tcpip_to_net_endian16(addr->port);
    header->length    = tcpip_to_net_endian16(size);
    header->checksum  = 0x0000;

    if (size > 0) {
        m = tcpip_allocate_mbuf();
        first->next = m;

        while (size > 0) {
            size_t copy_size = ((size >= TCPIP_MBUF_DATA_SIZE)? TCPIP_MBUF_DATA_SIZE : size);

            memcpy(m, p, copy_size);
            size -= copy_size;

            if (size > 0) {
                next = tcpip_allocate_mbuf();
                m->next = next;
                m = next;
            }
        }
    }

    return size;
}


void tcpip_receive_udp(struct tcpip_addr *src_addr, struct tcpip_addr *dest_addr,
                       const void *payload, size_t size) {

    struct tcpip_udp_header *header = (struct tcpip_udp_header *) payload;
    uint16_t src_port, dest_port, length, checksum;
    struct tcpip_socket *socket;

    src_port  = tcpip_to_host_endian16(header->src_port);
    dest_port = tcpip_to_host_endian16(header->dest_port);
    length    = tcpip_to_host_endian16(header->length);
    checksum  = tcpip_to_host_endian16(header->checksum);

    DEBUG("%d -> %d (length=%d)", src_port, dest_port, length);

    src_addr->port  = src_port;
    dest_addr->port = dest_port;

    socket = tcpip_get_socket(src_addr, dest_addr);

    if (!socket) {
	WARN("socket not found, ignoring");
	return;
    }

    tcpip_append_mbuf(&socket->rx, src_addr,
        (void *) ((uintptr_t) payload + sizeof(struct tcpip_udp_header)),
        size, TCPIP_MBUF_END);
}

