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
    NORETURN void (*update)(unsigned long deployment_id);
    void (*delay)(unsigned long ms);
    void (*dprint)(const char *msg, unsigned x);
    void (*printchar)(const char ch);
    int (*read_adc)(void);
    void (*analog_write)(int pin, int value);
    int (*gpio_read)(int pin);
    void (*gpio_write)(int pin, int data);
    void (*gpio_set_pin_mode)(int pin, int mode);
    int (*http_request)(const char *method, const char *url, size_t url_size,
                        const void *headers, size_t headers_size,
                        const void *payload, size_t payload_size,
                        void *buf, size_t buf_size, size_t *resp_size);
    const char *(*get_device_secret)(void);
    const char *(*get_server_url)(void);
    unsigned long (*get_deployment_id)(void);
    void (*accept_interrupt)(int pin, void (*cb)(int));
    void (*i2c_send)(int addr, const void *buf, size_t size);
    void (*i2c_receive)(int addr, void *buf, size_t size);
    unsigned long (*millis)(void);
};

extern struct firmware_info *finfo;

#endif
