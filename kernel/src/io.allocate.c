#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>

/** handles io.allocate */
void kernel_io_allocate(channel_t __ch, io_IOSpace iospace, uintptr_t addr, size_t size) {

    // TODO
    send_io_allocate_reply(__ch, OK, addr);
}
