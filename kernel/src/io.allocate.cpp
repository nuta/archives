#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include <hal.h>
#include "kernel.h"

/** handles io.allocate */
void kernel_io_allocate(channel_t __ch, io_space_t iospace, uintptr_t addr, size_t size) {
    uintptr_t vaddr;

    switch (iospace) {
    case IO_SPACE_PORT: // TODO: permit
        vaddr = addr;
	break;
    case IO_SPACE_MEM:
	vaddr = kernel_vmalloc(size);
        hal_link_page(&kernel_get_current_thread_group()->vm, vaddr,
	              size / PAGE_SIZE, addr, PAGE_PRESENT | PAGE_WRITABLE);
        DEBUG("allocated memory-mapped IO (v=%p, p=%p)", vaddr, addr);
	break;
    }

    send_io_allocate_reply(__ch, OK, vaddr);
}
