#ifndef __TCPIP_IP_H__
#define __TCPIP_IP_H__

#include "types.h"
#include "session.h"
#include "mbuf.h"

int tcpip_send_ip(struct tcpip_session *session,
                  struct tcpip_mbuf *payload, size_t size,
                  int flags, struct tcpip_addr *addr);

#endif
