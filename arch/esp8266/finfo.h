#ifndef __FINFO_H__
#define __FINFO_H__

typedef int ferr_t;
enum {
    BERR_OK       = 1,
    BERR_CONNECT  = 2,
    BERR_NOMEM    = 3,
    BERR_EOF      = 4,
};

struct firmware_info {
    void (*set_loop)(void (*func)());
    void (*dprint)(const char *msg, unsigned x);
    void (*printchar)(const char ch);
    int (*read_adc)();
    int (*gpio_read)(int pin);
    void (*gpio_write)(int pin, int data);
    void (*gpio_set_pin_mode)(int pin, int mode);
    ferr_t (*http_request)(const char *host, int port, const char *method,
                           const char *path, const void *headers, const void *payload,
                           size_t payload_size, void *buf, size_t buf_size);
};

extern struct firmware_info *finfo;

#endif
