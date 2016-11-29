#include <resea.h>
#include <resea/gpio.h>
#include <resea/interrupt.h>
#include <logging.h>
#include <kernel/thread.h>
#include <kernel/message.h>
#include <kernel/event.h>
#include <string.h>


const char *POINTER_TEST_STRING = "ABC123";

static payload_t msg1[] = {
    1,           // inline, null, null, null
    0x55aa55aa
};

static payload_t msg2[] = {
    1 | (0x03 << 4) | (0x04 << 8), // inline, pointer, pointer size, null
    0xabcd1234,
    0,
    0,
    0,
};


static channel_t server, client;
void client_thread(uintmax_t arg) {
    // create_thread() arg test
    TEST_EXPECT(arg == 0x55aa55aa, "create_thread() passes an correct argument");

    // send/recv test
    result_t r;
    r = send(client, &msg1, sizeof(msg1), 0);
    TEST_EXPECT(r == OK, "#1: send returns OK");

    payload_t buf[8];
    channel_t from;
    r = recv(client, (void *) &buf, sizeof(buf), 0, &from);
    TEST_EXPECT(r == OK,            "#2: recv returns OK");
    TEST_EXPECT(from == client,     "#2: sent from the server");
    TEST_EXPECT(buf[1] == msg2[1],  "#2: sent an inline payload correctly");
    TEST_EXPECT(!memcmp((const char *) buf[2], POINTER_TEST_STRING, buf[3]),
                "#2: sent a pointer payload correctly");
    TEST_EXPECT(buf[3] == msg2[3],  "#2: sent a pointer size payload correctly");

    listen_event(get_channel_by_cid(client), INTERRUPT_INTERRUPT0, 0);
    fire_event(INTERRUPT_INTERRUPT0);
    recv(client, (void *) &buf, sizeof(buf), 0, &from);
    TEST_EXPECT(from == 1, "sent a message from kernel");
    TEST_EXPECT(buf[0] == INTERRUPT_INTERRUPT0, "receive an event");

    // stub test
    INFO("client: entering stub tests");
    call_gpio_set_pin_mode(client, 13, GPIO_OUTPUT_PIN, &r);
    TEST_EXPECT(r == OK, "gpio.set_pin_mode returns OK");

    call_gpio_write(client, 13, GPIO_HIGH, &r);
    TEST_EXPECT(r == OK, "gpio.write returns OK");

    TEST_END();

    for (;;)
        yield();
}


static void test_messaging() {
    server = open();
    TEST_EXPECT(server != 0, "open() returns an channel ID");

    client = open();
    TEST_EXPECT(client != 0, "open() returns an channel ID");

    link(server, client);

    INFO("launching the client thread");
    struct thread *t;
    t = create_thread(get_current_thread()->process,
                      (uintptr_t) client_thread, 0x55aa55aa);
    TEST_EXPECT(t != NULL, "create_thread() returns non-NULL value");
    start_thread(t);

    // recv/send test
    result_t r;
    payload_t buf[4];
    channel_t reply_to;
    r = recv(server, (void *) &buf, sizeof(buf), 0, &reply_to);
    TEST_EXPECT(r == OK,            "server: recv returns OK");
    TEST_EXPECT(reply_to == server, "server: received from the client");
    TEST_EXPECT(buf[1] == msg1[1],  "server: received payloads correctly");

    msg2[2] = (payload_t) POINTER_TEST_STRING;
    msg2[3] = strlen(POINTER_TEST_STRING);
    send(reply_to, &msg2, sizeof(msg2), 0);

    INFO("server: I am now gpio server!");

    // stub test
    for (;;) {
        r = recv(server, (void *) &buf, sizeof(buf), 0, &reply_to);
        TEST_EXPECT(r == OK, "gpio_server: recv returns OK");

        switch (buf[1]) {
        case GPIO_SET_PIN_MODE:
            reply_gpio_set_pin_mode(reply_to, OK);
            break;
        case GPIO_WRITE:
            reply_gpio_write(reply_to, OK);
            break;
        }
    }
}


static void do_test_log_bufferring(char *str, char *expected_str) {
    char *buffer;
    size_t buffered_size;

    printfmt("");
    get_buffered_log(&buffer);
    get_buffered_log(&buffer); // do twice for the wrapped case
    printfmt_nonl("%s", str);
    buffered_size = get_buffered_log(&buffer);
    printfmt("");

    TEST_EXPECT(strlen(expected_str) == buffered_size,
                "get_buffered_log returns strlen(expected_str)");
    TEST_EXPECT(strncmp(buffer, expected_str, buffered_size) == 0,
                "get_buffered_log returns only filled lines");
}


static void test_log_bufferring() {
    do_test_log_bufferring("ABC\nDEFG\n", "ABC\nDEFG");
    do_test_log_bufferring("Hello!\nNew\nWorld!", "Hello!\nNew");
    do_test_log_bufferring("", "");
    do_test_log_bufferring("ABC", "");
}


void kernel_test_startup(void) {
    INFO("started kernel test");

    INFO("test_log_bufferring -------------------------");
    test_log_bufferring();
    INFO("test_messaging ------------------------------");
    test_messaging(); // MUST be last one
}
