#include <resea.h>
#include <logging.h>
#include <string.h>
#include <malloc.h>
#include <resea/channel.h>
#include <resea/gpio.h>
#include <resea/i2c.h>
#include <resea/timer.h>
#include <resea/interrupt.h>
#include <resea/makestack.h>
#include <resea/http.h>
#include "kernel/kmalloc.h"
#include "kernel/event.h"
#include "kernel/timer.h"
#include "kernel/message.h"
#include "arch/esp8266/finfo.h"


static void interrupt_handler(int pin) {

    fire_event(INTERRUPT_INTERRUPT0 + pin);
}


static void mainloop(channel_t server) {

    payload_t buf[16];
    channel_t reply_to;

    if(recv(server, (void *) &buf, sizeof(buf), 0, &reply_to) != OK) {
       return;
    }

    /* TODO: validate the pin number */

    switch (buf[1]) {
    case GPIO_SET_PIN_MODE: {
        DEBUG("gpio.set_pin_mode");
        int pin = buf[2];
        int mode = buf[3];
        finfo->gpio_set_pin_mode(pin, mode);
        send_gpio_set_pin_mode_reply(reply_to, OK, 0);
        break;
    }
    case GPIO_READ: {
        DEBUG("gpio.read");
        int pin = buf[2];
        send_gpio_read_reply(reply_to, OK, finfo->gpio_read(pin), 0);
        break;
    }
    case GPIO_READ_ANALOG: {
        DEBUG("gpio.read_analog");
        send_gpio_read_analog_reply(reply_to, OK, finfo->read_adc(), 0);
        break;
    }
    case GPIO_WRITE_ANALOG: {
        DEBUG("gpio.write_analog");
        int pin = buf[2];
        int value = buf[3];
        finfo->analog_write(pin, value);
        send_gpio_write_analog_reply(reply_to, OK, 0);
        break;
    }
    case GPIO_WRITE: {
        DEBUG("gpio.write");
        int pin = buf[2];
        int data = buf[3];
        finfo->gpio_write(pin, data);
        send_gpio_write_reply(reply_to, OK, 0);
        break;
    }
    case MAKESTACK_RESET:
        DEBUG("makestack.reset");
        finfo->update(-1); // never returns
        break;
    case MAKESTACK_UPDATE: {
        DEBUG("makestack.update");
        int deployment_id = buf[2];
        finfo->update(deployment_id); // never returns
        break;
    }
    case MAKESTACK_GET_DEVICE_SECRET: {
        DEBUG("makestack.get_device_secret");
        const char *device_secret = finfo->get_device_secret();
        size_t len = strlen(finfo->get_device_secret()) + 1;
        reply_makestack_get_device_secret(reply_to, (void *) device_secret, len);
        break;
    }
    case MAKESTACK_GET_SERVER_URL: {
        DEBUG("makestack.get_serve_url");
        const char *server_url = finfo->get_server_url();
        size_t len = strlen(finfo->get_server_url()) + 1;
        reply_makestack_get_server_url(reply_to, (void *) server_url, len);
        break;
    }
    case MAKESTACK_GET_DEPLOYMENT_ID: {
        DEBUG("makestack.get_deployment_id");
        int deployment_id = finfo->get_deployment_id();
        reply_makestack_get_deployment_id(reply_to, deployment_id);
        break;
    }
    // TODO: move this into the kernel
    case INTERRUPT_LISTEN: {
        channel_t channel = buf[2];
        int pin           = buf[3];
        payload_t arg     = buf[4];
        DEBUG("interrupt.listen %x", INTERRUPT_INTERRUPT0 + pin);
        listen_event(get_channel_by_cid(channel)->linked_to, INTERRUPT_INTERRUPT0 + pin, arg);
        finfo->accept_interrupt(pin, interrupt_handler);
        reply_interrupt_listen(reply_to, OK);
        break;
    }
    case I2C_SEND: {
        int addr    = buf[2];
        char *data  = (char *) buf[3];
        size_t size = buf[4];
        finfo->i2c_send(addr, data, size);
        reply_i2c_send(reply_to, OK);
        break;
    }
    case I2C_RECEIVE: {
        int addr    = buf[2];
        size_t size = buf[3];
        char *data  = (char *) malloc(size);
        finfo->i2c_receive(addr, data, size);
        reply_i2c_receive(reply_to, OK, data, size);

        free(data);
        break;
    }
    case TIMER_SET_INTERVAL: {
        channel_t channel;
        uint32_t  ms;
        uintmax_t arg;
        unmarshal_timer_set_interval((payload_t *) &buf, &channel, &ms, &arg);

        add_interval_timer(get_channel_by_cid(channel)->linked_to, ms, arg);
        reply_timer_set_interval(reply_to, OK);
        break;
    }
    case TIMER_SET_ONESHOT: {
        channel_t channel;
        uint32_t  ms;
        uintmax_t arg;
        unmarshal_timer_set_oneshot((payload_t *) &buf, &channel, &ms, &arg);

        add_oneshot_timer(get_channel_by_cid(channel)->linked_to, ms, arg);
        reply_timer_set_oneshot(reply_to, OK);
        break;
    }
    case HTTP_REQUEST: {
        void *url;
        size_t url_size;
        char *method;
        void *headers;
        size_t headers_size;
        void *payload;
        size_t payload_size;
        int options;
        int status_code;
        void *resp;
        size_t resp_size, received_size;
        static char *methods[] = { "GET", "POST", "DELETE", "PUT", "OPTIONS", "HEAD" };

        unmarshal_http_request((payload_t *) &buf, &options, &url, &url_size,
                               &headers, &headers_size, &payload, &payload_size);

        if ((options & 0xff) > 5) {
            // Invalid HTTP method.
            reply_http_request(reply_to, E_BAD_REQUEST, 0, NULL, 0);
            break;
        }

        method = methods[options & 0xff];

        if (options & HTTP_OPTION_DISCARD_RESPONSE_PAYLOAD) {
            resp = NULL;
        } else {
            resp_size = 1024;
            resp = malloc(resp_size);
        }

        status_code = finfo->http_request(method, url, url_size, headers, headers_size,
                          payload, payload_size, resp, resp_size, &received_size);

        reply_http_request(reply_to, OK, status_code, resp, received_size);

        free(resp);
        break;
    }
    default:
        WARN("esp82660-driver: unknown message (%p)", buf[1]);
    }

    discard(&buf, sizeof(buf));
}


extern int __stack;
extern int __stack_bottom;

void esp8266_driver_startup(void) {
    INFO("starting esp8266-driver");

    /* A stack area used in boot is no longer necessary. Reuse it. */
    add_kmalloc_chunk((void *) &__stack_bottom,
                      (uintptr_t) &__stack - (uintptr_t) &__stack_bottom,
                      false);

    channel_t server = open();
    channel_t channel_server = connect_to_local(1);
    result_t r;

    call_channel_register(channel_server, server, GPIO_INTERFACE, &r);
    call_channel_register(channel_server, server, I2C_INTERFACE, &r);
    call_channel_register(channel_server, server, TIMER_INTERFACE, &r);
    call_channel_register(channel_server, server, INTERRUPT_INTERFACE, &r);
    call_channel_register(channel_server, server, MAKESTACK_INTERFACE, &r);
    call_channel_register(channel_server, server, HTTP_INTERFACE, &r);

    INFO("esp8266-driver: ready");
    for (;;)
        mainloop(server);
}
