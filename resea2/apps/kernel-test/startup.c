#include <resea.h>
#include <resea/gpio.h>
#include <resea/interrupt.h>
#include <logging.h>
#include <kernel/kmalloc.h>
#include <kernel/thread.h>
#include <kernel/message.h>
#include <kernel/event.h>
#include <kernel/list.h>
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

    INFO("invoking an event %d", client);
    listen_event(get_channel_by_cid(client), INTERRUPT_INTERRUPT0, 0);
    fire_event(INTERRUPT_INTERRUPT0);
    recv(client, (void *) &buf, sizeof(buf), 0, &from);
    TEST_EXPECT(from == -1, "sent a message from kernel");
    TEST_EXPECT(buf[1] == INTERRUPT_INTERRUPT0, "receive an event");

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
                      (uintptr_t) client_thread, 0x55aa55aa, 0);
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


struct int_list {
    struct list *next;
    int a;
};

static void test_list() {
    struct int_list *l = NULL;

    struct int_list *e1 = (struct int_list *) malloc(sizeof(*e1));
    struct int_list *e2 = (struct int_list *) malloc(sizeof(*e2));
    struct int_list *e3 = (struct int_list *) malloc(sizeof(*e3));

    insert_into_list((struct list **) &l, e1);
    TEST_EXPECT(l == e1,
                "insert_into_list() appends an element to an emptry list");

    insert_into_list((struct list **) &l, e2);
    TEST_EXPECT(l->next == (struct list *) e2,
                "insert_into_list() appends an element to an list (e2)");
    TEST_EXPECT(l->next->next == NULL,
                "insert_into_list() set the terminal `next' to NULL (e2)");

    insert_into_list((struct list **) &l, e3);
    TEST_EXPECT(l->next->next == (struct list *) e3,
                "insert_into_list() appends an element to an list (e3)");
    TEST_EXPECT(l->next->next->next == NULL,
                "insert_into_list() set the terminal `next' to NULL (e3)");

    remove_from_list((struct list **) &l, e2);
    TEST_EXPECT(l == e1 && l->next == (struct list *) e3 && l->next->next == NULL,
                "remove_from_list() can removes an element in middle");

    remove_from_list((struct list **) &l, e3);
    TEST_EXPECT(l == e1 && l->next == NULL,
                "remove_from_list() can removes the temrinal element");

    remove_from_list((struct list **) &l, e1);
    TEST_EXPECT(l == NULL,
                "remove_from_list() can removes the head element");

    remove_from_list((struct list **) &l, e1);
    TEST_EXPECT(l == NULL,
                "remove_from_list() do nothing if list is empty");

    insert_into_list((struct list **) &l, e1);
    TEST_EXPECT(l == e1 && l->next == NULL,
                "insert_into_list() appends an element to an emptry list #1");
}


void test_kmalloc(void) {
    // Create a free'd chunk before ptr1.
    kfree(kmalloc(5, KMALLOC_NORMAL));

    size_t remaining = get_remaining_memory();
    void *ptr1 = kmalloc(1024, KMALLOC_NORMAL);
    void *ptr2 = kmalloc(1024, KMALLOC_NORMAL);
    void *ptr3 = kmalloc(1024, KMALLOC_NORMAL);
    kfree(ptr2);
    kfree(ptr1);
    void *ptr4 = kmalloc(1024, KMALLOC_NORMAL);
    kfree(ptr4);
    kfree(ptr3);

    TEST_EXPECT(ptr1 && ptr2 && ptr3 && ptr4,
                "kmalloc does not return NULL pointers");
    TEST_EXPECT(get_remaining_memory() - remaining == 0,
                "kmalloc free pointers without memory leaks");
}


static void test_printfmt(void) {
    union {uint32_t u; float f; }  f1 = {0x7f800000}; // Inf
    union {uint32_t u; float f; }  f2 = {0xff800000}; // -Inf
    union {uint64_t u; double f; } f3 = {0xffffffffffffffff}; // NaN

    DEBUG("%s:%d %c %f %f %f %f %d %d\n", __func__, __LINE__, 'A', 12.456, 0., -0.33, 0.1, 123, -123);
    DEBUG("%s:%d %f %f %f %f %f\n", __func__, __LINE__,  0.0001, 12345678.9, f1.f, f2.f, f3.f);
}


void kernel_test_startup(void) {
    INFO("started kernel test");

    INFO("test_list -----------------------------------");
    test_list();
    INFO("test_printfmt -------------------------------");
    test_printfmt();
    INFO("test_log_bufferring -------------------------");
    test_log_bufferring();
    INFO("test_kmalloc --------------------------------");
    test_kmalloc();
    INFO("test_messaging ------------------------------");
    test_messaging(); // MUST be last one
}
