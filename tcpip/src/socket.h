#ifndef __TCPIP_SOCKET_H__
#define __TCPIP_SOCKET_H__

#include "types.h"
#include "mbuf.h"
#include <resea/tcpip.h>

struct socket {
    ident_t id;
    int used;

    struct addr local_addr;
    struct addr remote_addr;

    channel_t handler;
    tcpip_protocol_t protocol;

    // Transmit (TX) and queues
    struct mbuf *tx;
};


struct socket *tcpip_get_socket_by_id(ident_t id);
struct socket *tcpip_get_socket_by_addr(struct addr *remote_addr,
                                      struct addr *local_addr);
result_t tcpip_bind_socket(struct socket *socket, struct addr *addr);

ident_t tcpip_create_socket();
void tcpip_destroy_socket(struct socket *socket);

void tcpip_init_socket();

#endif
