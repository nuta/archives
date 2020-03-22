#ifndef __SERVER_H__
#define __SERVER_H__

#include <types.h>

struct message;
error_t server_mainloop(cid_t ch, error_t (*process)(struct message *m));
error_t server_mainloop_with_deferred(cid_t ch,
                                      error_t (*process)(struct message *m),
                                      error_t (*deferred_work)(void));
error_t server_register(cid_t discovery_server, cid_t server_ch,
                        uint16_t interface);
error_t server_connect(cid_t discovery_server, uint16_t interface, cid_t *ch);

#endif
