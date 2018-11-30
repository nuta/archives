#ifndef __KERNEL_MEMORY_H__
#define __KERNEL_MEMORY_H__

#include <resea.h>

namespace kernel {
namespace memory {

// Physical memory space
struct pm_space {
    mutex_t    lock;
    size_t     num;        // the number of pages
    paddr_t    paddr;      // the beginning address of the space
    uint8_t    *pages;     // reference counts
    struct pm_space *next; // the next physical memory space if it is not nullptr
};

size_t get_dynamic_vpages_num(void);
uintptr_t vmalloc(size_t size);
void *allocate(size_t size, uint32_t flags);
result_t allocate_at(paddr_t at, size_t size, uint32_t flags,
                     uintptr_t *addr, paddr_t *paddr);
void release(void *p);
void init();
void page_fault_handler(uintptr_t addr, uint32_t reason);

} // namespace memory
} // namespace kernel

#endif
