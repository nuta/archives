#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <hal.h>


/** handles io.write8 */
void kernel_io_write8(channel_t __ch, io_IOSpace iospace, uintptr_t addr,
                    offset_t offset, uint8_t data) {

    hal_io_write8(iospace, addr, offset, data);
    send_io_write8_reply(__ch, OK);
}
