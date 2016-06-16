#include "../src/kernel.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/datetime.h>


extern "C" void kernel_test(void) {
    channel_t client;
    result_t r;

    //
    //  messaging test
    //

    // connect to the server
    client = create_channel();
    resea::interfaces::channel::call_connect(connect_to_local(1), client,
        INTERFACE(datetime), &r);

    resea::interfaces::datetime::call_delay(client, 0x55aa, &r);
    TEST_EXPECT_DESC("call: receive return values", r == OK);

    TEST_END();
}
