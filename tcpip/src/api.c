#include "api.h"
#include "udp.h"


struct tcpip_session *tcpip_open(struct tcpip_instance *instance) {

    return tcpip_create_session(instance);
}


void tcpip_close(struct tcpip_session *session) {

}



int tcpip_bind(struct tcpip_session *session, struct tcpip_addr *addr) {

    return tcpip_bind_session(session, addr);
}


size_t tcpip_sendto(struct tcpip_session *session, const void *buf, size_t size, int flags,
                    struct tcpip_addr *addr) {

    return tcpip_send_udp(session, buf, size, flags, addr);
}


size_t tcpip_recvfrom(struct tcpip_session *session, void *buf, size_t size, int flags,
                      struct tcpip_addr *addr) {

    return tcpip_pop_mbuf(&session->rx, buf, size, flags, addr);
}

