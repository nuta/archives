#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <hal.h>


/** handles io.write64 */
void kernel_io_write64(channel_t __ch, io_IOSpace iospace, uintptr_t addr,
                     offset_t offset, uint64_t data) {

    send_io_write64_reply(__ch, E_NOTSUPPORTED); /* TODO */
}
