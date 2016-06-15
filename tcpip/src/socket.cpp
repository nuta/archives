#include "tcpip.h"
#include "socket.h"
#include "ipv4.h"
#include "printf.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/tcpip.h>
#include <string.h>

using namespace tcpip;

namespace tcpip {

static struct socket *sockets;
static size_t sockets_max;

static bool compare_addr(struct addr *addr1, struct addr *addr2) {

    if (addr1->port != PORT_ANY && addr2->port != PORT_ANY) {
        if (addr1->port != addr2->port)
            return false;
    }

    if (addr1->protocol & TCPIP_PROTOCOL_IPV4 && addr2->protocol & TCPIP_PROTOCOL_IPV4) {
        if (addr1->ipv4_addr != IPV4_ADDR_ANY &&
            addr2->ipv4_addr != IPV4_ADDR_ANY) {
            if (addr1->ipv4_addr != addr2->ipv4_addr) {
                return true;
            }
        }
    } else {
        WARN("addr->protcol is invalid (addr1->protocol=%#x, addr2->protocol=%#x)",
            addr1->protocol, addr2->protocol);
        return false;
    }

    return true;
}


struct socket *get_socket_by_id(ident_t id) {

    return &sockets[id];
}


struct socket *get_socket_by_addr(struct addr *remote_addr,
                                        struct addr *local_addr) {

    for (size_t i = 0; i < sockets_max; i++) {
        struct socket *s = &sockets[i];
        if (s->used &&
            compare_addr(remote_addr, &s->remote_addr) &&
            compare_addr(local_addr,  &s->local_addr)) {

            return s;
        }
    }

    return nullptr;
}


void destroy_socket(struct socket *socket) {
    // TODO
}


// TODO: return result_t
ident_t create_socket() {

    for (ident_t i = 1; i < sockets_max; i++) {
        struct socket *s = &sockets[i];

        if (!s->used) {
            // TODO: lock
            s->used = 1;
            s->id   = i;
            s->tx   = nullptr;
            DEBUG("created a new socket #%d", i);
            return i;
        }
    }

    WARN("failed to allocate a new socket");
    return 0;
}


result_t bind_socket(struct socket *socket, struct addr *addr) {
    struct addr default_remote_addr;

    default_remote_addr.protocol  = addr->protocol;
    default_remote_addr.port      = PORT_ANY;
    default_remote_addr.ipv4_addr = IPV4_ADDR_ANY;

    // TODO: support IPV6

    // TODO: check conflicts with exsisting sockets
    memcpy(&socket->local_addr, addr, sizeof(*addr));
    memcpy(&socket->remote_addr, &default_remote_addr, sizeof(default_remote_addr));

    return OK;
}


void init_socket() {

    sockets     = (struct socket *) allocate_memory(sizeof(struct socket) * 256,
                                                    MEMORY_ALLOC_NORMAL);
    sockets_max = 256;
}

} // namespace tcpip
