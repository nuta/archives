#include <resea/cpp/memory.h>
#include <resea/tcpip.h>
#include <assert.h>
#include <string.h>
#include "ip.h"
#include "udp.h"
#include "endian.h"
#include "socket.h"
#include "printf.h"


result_t tcpip_send_udp(struct socket *socket,
                        struct mbuf *mbuf,
                        int flags, struct addr *addr) {

    struct mbuf *header_mbuf;
    struct tcpip_udp_header *header;

    assert(MBUF_DATA_SIZE >= sizeof(struct tcpip_udp_header));

    header_mbuf = tcpip_allocate_mbuf();
    header_mbuf->length = sizeof(struct tcpip_udp_header);

    header = (struct tcpip_udp_header *) &header_mbuf->data;
    header->src_port  = tcpip_to_net_endian16(socket->local_addr.port);
    header->dest_port = tcpip_to_net_endian16(addr->port);
    header->length    = tcpip_to_net_endian16(mbuf->total_length);
    header->checksum  = 0x0000;

    tcpip_append_mbuf(header_mbuf, mbuf, false);
    return tcpip_send_ip(socket, header_mbuf, IPTYPE_UDP, flags, addr);
}


void tcpip_receive_udp(struct addr *src_addr, struct addr *dest_addr,
                       struct mbuf *mbuf) {

    struct tcpip_udp_header header;
    uint16_t src_port, dest_port, length, checksum;
    struct socket *socket;

    if (tcpip_copy_from_mbuf(&header, mbuf, sizeof(header)) != OK) {
        DEBUG("too short packet");
        return;
    }

    src_port  = tcpip_to_host_endian16(header.src_port);
    dest_port = tcpip_to_host_endian16(header.dest_port);
    length    = tcpip_to_host_endian16(header.length);
    checksum  = tcpip_to_host_endian16(header.checksum);

    DEBUG("received UDP: %d -> %d (length=%d)", src_port, dest_port, length);

    src_addr->port  = src_port;
    dest_addr->port = dest_port;

    socket = tcpip_get_socket_by_addr(src_addr, dest_addr);

    if (!socket) {
	WARN("socket not found, ignoring");
	return;
    }

    void *payload = allocate_memory(length, MEMORY_ALLOC_NORMAL);
    tcpip_copy_from_mbuf(payload, mbuf, length);

    sendas_tcpip_received(socket->handler,
        socket->id,          PAYLOAD_INLINE,
        TCPIP_PROTOCOL_IPV4, PAYLOAD_INLINE,
        (void *) "",         PAYLOAD_OOL,
        0,                   PAYLOAD_INLINE, // TODO
        src_port,            PAYLOAD_INLINE,
        payload,             PAYLOAD_MOVE_OOL,
        length,              PAYLOAD_INLINE);
}

