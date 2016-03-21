#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <hal.h>


/** handles io.write16 */
void kernel_io_write16(channel_t __ch, io_IOSpace iospace,
                     uintptr_t addr, offset_t offset, uint16_t data) {

    hal_io_write16(iospace, addr, offset, data);
    send_io_write16_reply(__ch, OK);
}
