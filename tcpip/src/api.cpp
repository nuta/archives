#include "api.h"
#include "udp.h"


struct tcpip_socket *tcpip_open() {

    return tcpip_create_socket();
}


void tcpip_close(struct tcpip_socket *socket) {

}



int tcpip_bind(struct tcpip_socket *socket, struct tcpip_addr *addr) {

    return tcpip_bind_socket(socket, addr);
}


size_t tcpip_sendto(struct tcpip_socket *socket, const void *buf, size_t size, int flags,
                    struct tcpip_addr *addr) {

    return tcpip_send_udp(socket, buf, size, flags, addr);
}


size_t tcpip_recvfrom(struct tcpip_socket *socket, void *buf, size_t size, int flags,
                      struct tcpip_addr *addr) {

    return tcpip_pop_mbuf(&socket->rx, buf, size, flags, addr);
}

