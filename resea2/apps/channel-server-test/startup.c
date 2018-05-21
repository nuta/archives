#include <resea.h>
#include <resea/gpio.h>
#include <resea/channel.h>
#include <kernel/thread.h>
#include <logging.h>


static channel_t gpio_server;

void gpio_server_thread(void) {

    payload_t buf[4];
    channel_t reply_to;

    recv(gpio_server, (void *) &buf, sizeof(buf), 0, &reply_to);
    TEST_EXPECT(buf[2] == 13, "receive the correct pin number");
    send_gpio_write_reply(reply_to, OK, 0);

    for (;;)
        yield();
}


void channel_server_test_startup(void) {
    INFO("starting channel server tests");

    gpio_server = open();
    if (!gpio_server) {
        WARN("channel-server-test: failed to open a channel");
        return;
    }

    channel_t gpio_client = open();
    if (!gpio_client) {
        WARN("channel-server-test: failed to open a channel");
        return;
    }

    channel_t channel_server = connect_to_local(1);

    INFO("launching the gpio_server thread");
    struct thread *t;
    t = create_thread(get_current_thread()->process,
                      (uintptr_t) gpio_server_thread, 0x55aa55aa);
    TEST_EXPECT(t != NULL, "create_thread() returns non-NULL value");
    start_thread(t);

    result_t r;
    call_channel_register(channel_server, gpio_server, GPIO_INTERFACE, &r);
    TEST_EXPECT(r == OK, "register: channel-server returns OK");

    call_channel_connect(channel_server, gpio_client, GPIO_INTERFACE, &r);
    TEST_EXPECT(r == OK, "connect: channel-server returns OK");

    call_gpio_write(gpio_client, 13, GPIO_HIGH, &r);
    TEST_EXPECT(r == OK, "gpio-server returns OK");
    TEST_END();
}
