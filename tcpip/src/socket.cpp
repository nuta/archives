#include <string.h>
#include "socket.h"
#include "malloc.h"
#include "ipv4.h"
#include "printf.h"

static struct tcpip_socket *sockets;
static size_t sockets_max;

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

struct tcpip_socket *tcpip_get_socket(struct tcpip_addr *remote_addr,
                                      struct tcpip_addr *local_addr) {

    for (size_t i = 0; i < sockets_max; i++) {
        struct tcpip_socket *s = &sockets[i];
        if (s->used &&
            compare_addr(remote_addr, &s->remote_addr) &&
            compare_addr(local_addr,  &s->local_addr)) {

            return s;
        }
    }

    return nullptr;
}


void tcpip_destroy_socket(struct tcpip_socket *socket) {
    // TODO
}


struct tcpip_socket *tcpip_create_socket() {

    for (size_t i = 0; i < sockets_max; i++) {
        struct tcpip_socket *s = &sockets[i];

        if (!s->used) {
            // TODO: lock
            s->used = 1;
            s->rx.first    = nullptr;
            s->rx.last     = nullptr;
            s->tx.first    = nullptr;
            s->tx.last     = nullptr;
            return s;
        }
    }

    return nullptr;
}


/* Retuns 0 on success */
int tcpip_bind_socket(struct tcpip_socket *socket, struct tcpip_addr *addr) {
    struct tcpip_addr default_remote_addr;

    default_remote_addr.protocol  = addr->protocol;
    default_remote_addr.port      = TCPIP_PORT_ANY;
    default_remote_addr.ipv4_addr = TCPIP_IPV4_ADDR_ANY; // TODO: support IPV6

    // TODO: check conflicts with exsisting sockets
    memcpy(&socket->local_addr, addr, sizeof(*addr));
    memcpy(&socket->remote_addr, &default_remote_addr, sizeof(default_remote_addr));

    return 0;
}


void tcpip_init_socket() {

    sockets     = (struct tcpip_socket *) tcpip_malloc(sizeof(struct tcpip_socket) * 256);
    sockets_max = 256;
}
