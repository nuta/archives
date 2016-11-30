#ifndef __FINFO_H__
#define __FINFO_H__

#include <types.h>

typedef int ferr_t;
enum {
    BERR_OK       = 1,
    BERR_CONNECT  = 2,
    BERR_NOMEM    = 3,
    BERR_EOF      = 4,
};

struct firmware_info {
    void (*start_loop)(void);
    NORETURN void (*update)(unsigned long deployment_id);
    void (*set_interval)(int ms, void (*callback)(void));
    void (*dprint)(const char *msg, unsigned x);
    void (*printchar)(const char ch);
    int (*read_adc)(void);
    int (*gpio_read)(int pin);
    void (*gpio_write)(int pin, int data);
    void (*gpio_set_pin_mode)(int pin, int mode);
    ferr_t (*http_request)(const char *host, int port, const char *method,
                           const char *path, const void *headers, const void *payload,
                           size_t payload_size, void *buf, size_t buf_size, bool tls);
    const char *(*get_device_secret)(void);
    const char *(*get_server_url)(void);
    unsigned long (*get_deployment_id)(void);
    void (*accept_interrupt)(int pin, void (*cb)(int));
};

extern struct firmware_info *finfo;

#endif
