#ifndef __TCPIP_SOCKET_H__
#define __TCPIP_SOCKET_H__

#include "types.h"
#include "mbuf.h"
#include <resea/tcpip.h>


namespace tcpip {

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


struct socket *get_socket_by_id(ident_t id);
struct socket *get_socket_by_addr(struct addr *remote_addr,
                                  struct addr *local_addr);
result_t bind_socket(struct socket *socket, struct addr *addr);

ident_t create_socket();
void destroy_socket(struct socket *socket);

void init_socket();

} // namespace tcpip

#endif
