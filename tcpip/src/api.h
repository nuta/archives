#ifndef __TCPIP_API_H__
#define __TCPIP_API_H__

#include "types.h"
#include "socket.h"

struct tcpip_socket *tcpip_open();
void tcpip_close(struct tcpip_socket *socket);
int tcpip_bind(struct tcpip_socket *socket, struct tcpip_addr *addr);
size_t tcpip_sendto(struct tcpip_socket *socket, const void *buf, size_t size, int flags,
     struct tcpip_addr *addr);
size_t tcpip_recvfrom(struct tcpip_socket *socket, void *buf, size_t size, int flags,
                      struct tcpip_addr *addr);

#endif
