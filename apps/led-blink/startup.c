#include <resea.h>
#include <logging.h>
#include <resea/channel.h>
#include <resea/gpio.h>
#include "arch/esp8266/finfo.h"


channel_t gpio_server;
void my_loop() {
    result_t r;
    static int x = 0;

    if (x % 2 == 0) {
        call_gpio_write(gpio_server, 4, GPIO_HIGH, &r);
    } else {
        call_gpio_write(gpio_server, 4, GPIO_LOW, &r);
    }

    for (volatile int i=0; i < 0x100000; i++);
    x++;
}


void led_blink_startup(void) {
    INFO("starting LED blinker");

    gpio_server = open(); // TODO: check return value
    channel_t channel_server = connect_to_local(1);

    result_t r;
    call_channel_connect(channel_server, gpio_server, GPIO_INTERFACE, &r);

    call_gpio_set_pin_mode(gpio_server, 4, GPIO_OUTPUT_PIN, &r);
    call_gpio_write(gpio_server, 4, GPIO_HIGH, &r);

    INFO("led-blink: turned a LED on!");
    finfo->set_loop(my_loop);
    INFO("set loop");
    for (;;);
}
