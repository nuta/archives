#ifndef __TCPIP_PRINTF_H__
#define __TCPIP_PRINTF_H__

#define FMT_MACADDR   "%02x:%02x:%02x:%02x:%02x:%02x"
#define FMT_IPV4ADDR  "%u.%u.%u.%u"

#define FMTARG_MACADDR(x)  (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5]
#define FMTARG_IPV4ADDR(x) ((x) >> 24) & 0xff, ((x) >> 16) & 0xff, ((x) >> 8)  & 0xff, ((x) >> 0)  & 0xff

#endif
