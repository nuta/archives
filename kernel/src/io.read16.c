#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <hal.h>


/** handles io.read16 */
void kernel_io_read16(channel_t __ch, io_IOSpace iospace, uintptr_t addr, offset_t offset) {
    uint8_t data;

    data = hal_io_read16(iospace, addr, offset);
    send_io_read16_reply(__ch, OK, data);
}
