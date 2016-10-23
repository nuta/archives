#include <resea.h>
#include <logging.h>
#include <resea/channel.h>
#include <resea/gpio.h>
#include "arch/esp8266/finfo.h" // XXX


static void mainloop(channel_t server) {

    payload_t buf[4];
    channel_t reply_to;
    
    if(recv(server, (void *) &buf, sizeof(buf), 0, &reply_to) != OK) {
       return;
    }

    /* TODO: validate the pin number */

    int mode, data;
    int pin = buf[2];
    switch (buf[1]) {
    case GPIO_SET_PIN_MODE:
        DEBUG("gpio.set_pin_mode");
        mode = buf[3];
        finfo->gpio_set_pin_mode(pin, mode);
        send_gpio_set_pin_mode_reply(reply_to, OK, 0);
        break;
    case GPIO_READ:
        DEBUG("gpio.read");
        send_gpio_read_reply(reply_to, OK, finfo->gpio_read(pin), 0);
        break;
    case GPIO_READ_ANALOG:
        DEBUG("gpio.read_analog");
        send_gpio_read_analog_reply(reply_to, OK, finfo->read_adc(), 0);
        break;
    case GPIO_WRITE:
        DEBUG("gpio.write");
        data = buf[3];
        finfo->gpio_write(pin, data);
        send_gpio_write_reply(reply_to, OK, 0);
        break;
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
    
    INFO("esp8266-driver: ready");
    for (;;)
        mainloop(server);
}
