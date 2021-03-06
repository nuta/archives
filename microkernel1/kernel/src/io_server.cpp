#include "kernel.h"
#include "memory.h"
#include "thread.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include <hal.h>


namespace kernel {
namespace io_server {

/** handles io.allocate */
void handle_allocate(channel_t __ch, resea::interfaces::io::space_t iospace,
                     uintptr_t addr, size_t size) {
    uintptr_t vaddr;

    switch (iospace) {
    case resea::interfaces::io::SPACE_PORT: // TODO: permit
        vaddr = addr;
        break;
    case resea::interfaces::io::SPACE_MEM:
        vaddr = memory::vmalloc(size);
        hal_link_page(&thread::get_current_thread_group()->vm, vaddr,
                      size / PAGE_SIZE, addr, PAGE_PRESENT | PAGE_WRITABLE);
        DEBUG("allocated memory-mapped IO (v=%p, p=%p)", vaddr, addr);
        break;
    }

    resea::interfaces::io::send_allocate_reply(__ch, OK, vaddr);
}


/** handles io.release */
void handle_release(channel_t __ch, resea::interfaces::io::space_t iospace,
                    uintptr_t addr) {

    // TODO
}

} // namespace io_server
} // namespace kernel
