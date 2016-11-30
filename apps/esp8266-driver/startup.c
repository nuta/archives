#include <resea.h>
#include <logging.h>
#include <string.h>
#include <resea/channel.h>
#include <resea/gpio.h>
#include <resea/interrupt.h>
#include <resea/makestack.h>
#include "kernel/event.h"
#include "arch/esp8266/finfo.h"


static void interrupt_handler(int pin) {

    fire_event(INTERRUPT_INTERRUPT0 + pin);
}


static void mainloop(channel_t server) {

    payload_t buf[4];
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
        DEBUG("interrupt.accept");
        channel_t channel = buf[2];
        int pin           = buf[3];
        listen_event(get_channel_by_cid(channel), INTERRUPT_INTERRUPT0 + pin, 0);
        finfo->accept_interrupt(pin, interrupt_handler);
        reply_interrupt_listen(reply_to, OK);
        break;
    }
    default:
        WARN("esp82660-driver: unknown message (%p)", buf[1]);
    }
}


void esp8266_driver_startup(void) {
    INFO("starting esp8266-driver");

    channel_t server = open();

    channel_t channel_server = connect_to_local(1);
    result_t r;

    call_channel_register(channel_server, server, GPIO_INTERFACE, &r);
    call_channel_register(channel_server, server, MAKESTACK_INTERFACE, &r);

    INFO("esp8266-driver: ready");
    for (;;)
        mainloop(server);
}
