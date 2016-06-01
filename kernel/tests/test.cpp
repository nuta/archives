#include "../src/kernel.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/datetime.h>


void kernel_test(void) {
    channel_t client;
    result_t r;

    //
    //  messaging test
    //

    // connect to the server
    client = create_channel();
    call_channel_connect(connect_to_local(1), client,
        INTERFACE(datetime), &r);

    call_datetime_delay(client, 0x55aa, &r);
    TEST_EXPECT_DESC("call: receive return values", r == OK);

    TEST_END();
}
