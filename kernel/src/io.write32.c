#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <hal.h>


/** handles io.write32 */
void kernel_io_write32(channel_t __ch, io_IOSpace iospace, uintptr_t addr,
                     offset_t offset, uint32_t data) {

    hal_io_write32(iospace, addr, offset, data);
    send_io_write32_reply(__ch, OK);
}
