#include "kernel.h"
#include "memory.h"
#include "thread.h"
#include <resea.h>
#include <resea/pager.h>


namespace kernel {
namespace page_fault {

static void handler(uintptr_t addr, uint32_t reason) {
    struct thread::thread_group *g;

    if(!(reason & PGFAULT_USER)) {
        PANIC("page fault in the kernel mode: addr=%p, reason=%#8x",
              addr, reason);
    }

    g = thread::get_current_thread_group();

    for (size_t i=0; i< g->vm.areas_num; i++) {
        struct vm_area *area = &g->vm.areas[i];

        if (area->addr <= addr && addr < area->addr + area->size) {
            // found the vm_area
            offset_t offset; // offset from the area
            result_t result;
            uintptr_t aligned_addr, filled_addr;
            size_t filled_size;

            INFO("page_fault_handler: found a vm_area, filling...");

            aligned_addr = addr & ~(PAGE_SIZE - 1);
            offset = aligned_addr - area->addr;
            resea::interfaces::pager::call_fill(area->pager,
                 area->pager_arg, area->offset + offset, PAGE_SIZE,
                 &result, (void **) &filled_addr, &filled_size);

            INFO("linking v=%p, p=%p, filled_addr=%p", aligned_addr,
                 hal_vaddr_to_paddr(&g->vm, filled_addr), filled_addr);

            hal_link_page(&g->vm, aligned_addr,
                          hal_vaddr_to_paddr(&g->vm, filled_addr), 1,
                          PAGE_PRESENT | PAGE_READABLE | PAGE_WRITABLE |
                          PAGE_USER | PAGE_EXECUTABLE /* XXX: use area->flags and
                                                  verify `reason` */);
            INFO("page_fault_handler: filled, resuming");
            return;
        }
    }

    PANIC("page fault at an unmapped area: addr=%p, reason=%#8x",
          addr, reason);
}


void init() {

    hal_set_callback(HAL_CALLBACK_PAGE_FAULT, (void *) handler);
}

} // namespace page_fault
} // namespace kernel
