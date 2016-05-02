#ifndef __TCPIP_SESSION_H__
#define __TCPIP_SESSION_H__

#include "types.h"
#include "mbuf.h"

struct tcpip_session {
    int used;

    struct tcpip_instance *instance;

    struct tcpip_addr local_addr;
    struct tcpip_addr remote_addr;

    // Transmit (TX) and Receive (RX) queues
    struct tcpip_mqueue tx;
    struct tcpip_mqueue rx;
};


struct tcpip_session *tcpip_get_session(struct tcpip_instance *instance,
                                        struct tcpip_addr *remote_addr,
                                        struct tcpip_addr *local_addr);
int tcpip_bind_session(struct tcpip_session *session, struct tcpip_addr *addr);

struct tcpip_session *tcpip_create_session(struct tcpip_instance *instance);
void tcpip_destroy_session(struct tcpip_session *session);

#endif
