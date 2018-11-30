#ifndef __SERVER_H__
#define __SERVER_H__

#include "list.h"

struct client {
    struct client *next;
    u32_t service_type;
    channel_t ch;
};

struct service {
    struct service *next;
    u32_t service_type;
    channel_t server;
};

DEFINE_LIST(service, struct service)
DEFINE_LIST(client, struct client)

extern struct channel *kernel_channel;

void kernel_server(void);
void kernel_server_init(void);

#endif
