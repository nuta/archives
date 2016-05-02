#ifndef __TCPIP_API_H__
#define __TCPIP_API_H__

#include "types.h"
#include "instance.h"
#include "session.h"

struct tcpip_session *tcpip_open(struct tcpip_instance *instance);
void tcpip_close(struct tcpip_session *session);
int tcpip_bind(struct tcpip_session *session, struct tcpip_addr *addr);
size_t tcpip_sendto(struct tcpip_session *session, const void *buf, size_t size, int flags,
     struct tcpip_addr *addr);
size_t tcpip_recvfrom(struct tcpip_session *session, void *buf, size_t size, int flags,
                      struct tcpip_addr *addr);

#endif
