#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <hal.h>


/** handles io.read64 */
void kernel_io_read64(channel_t __ch, io_IOSpace iospace, uintptr_t addr,  offset_t offset) {

    send_io_read64_reply(__ch, E_NOTSUPPORTED, 0); /* TODO */
}
