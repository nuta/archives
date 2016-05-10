#ifndef __TCPIP_SOCKET_H__
#define __TCPIP_SOCKET_H__

#include "types.h"
#include "mbuf.h"

struct tcpip_socket {
    int used;

    struct tcpip_addr local_addr;
    struct tcpip_addr remote_addr;

    // Transmit (TX) and Receive (RX) queues
    struct tcpip_mqueue tx;
    struct tcpip_mqueue rx;
};


struct tcpip_socket *tcpip_get_socket(struct tcpip_addr *remote_addr,
                                      struct tcpip_addr *local_addr);
int tcpip_bind_socket(struct tcpip_socket *socket, struct tcpip_addr *addr);

struct tcpip_socket *tcpip_create_socket();
void tcpip_destroy_socket(struct tcpip_socket *socket);

void tcpip_init_socket();

#endif
