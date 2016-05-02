#include <string.h>
#include "session.h"
#include "ipv4.h"
#include "printf.h"


static int compare_addr(struct tcpip_addr *addr1, struct tcpip_addr *addr2) {

    if (addr1->port != TCPIP_PORT_ANY && addr2->port != TCPIP_PORT_ANY) {
        if (addr1->port != addr2->port)
            return 0;
    }

    if (addr1->protocol & TCPIP_IPV4 && addr2->protocol & TCPIP_IPV4) {
        if (addr1->ipv4_addr != TCPIP_IPV4_ADDR_ANY &&
            addr2->ipv4_addr != TCPIP_IPV4_ADDR_ANY) {

            if (addr1->ipv4_addr != addr2->ipv4_addr) {
                return 0;
            }
        }
    } else {
        WARN("addr->protcol is invalid (addr1->protocol=%#x, addr2->protocol=%#x)",
            addr1->protocol, addr2->protocol);
        return 0;
    }

    return 1;
}

struct tcpip_session *tcpip_get_session(struct tcpip_instance *instance,
                                        struct tcpip_addr *remote_addr,
                                        struct tcpip_addr *local_addr) {

    for (int i = 0; i < instance->sessions_max; i++) {
        struct tcpip_session *s = &instance->sessions[i];
        if (s->used &&
            compare_addr(remote_addr, &s->remote_addr) &&
            compare_addr(local_addr,  &s->local_addr)) {

            return s;
        }
    }

    return NULL;
}


void tcpip_destroy_session(struct tcpip_session *session) {
    // TODO
}


struct tcpip_session *tcpip_create_session(struct tcpip_instance *instance) {

    for (int i = 0; i < instance->sessions_max; i++) {
        struct tcpip_session *s = &instance->sessions[i];

        if (!s->used) {
            // TODO: lock
            s->used = 1;
            s->instance    = instance;
            s->rx.instance = instance;
            s->rx.first    = NULL;
            s->rx.last     = NULL;
            s->tx.instance = instance;
            s->tx.first    = NULL;
            s->tx.last     = NULL;
            return s;
        }
    }

    return NULL;
}


/* Retuns 0 on success */
int tcpip_bind_session(struct tcpip_session *session, struct tcpip_addr *addr) {
    struct tcpip_addr default_remote_addr;

    default_remote_addr.protocol  = addr->protocol;
    default_remote_addr.port      = TCPIP_PORT_ANY;
    default_remote_addr.ipv4_addr = TCPIP_IPV4_ADDR_ANY; // TODO: support IPV6

    // TODO: check conflicts with exsisting sessions
    memcpy(&session->local_addr, addr, sizeof(*addr));
    memcpy(&session->remote_addr, &default_remote_addr, sizeof(default_remote_addr));

    return 0;
}

