#include "string.h"
#include "printk.h"
#include "types.h"
#include "thread.h"
#include "process.h"
#include "memory.h"

paddr_t allocated;
int used = 0;
paddr_t alloc_pages(size_t size, UNUSED int flags) {
    size = ROUND_UP(size, PAGE_SIZE);
    paddr_t addr = allocated;
    allocated += size;

    // XXX: The page could be not mapped.
    if (flags & KMALLOC_ZEROED) {
        memset(from_paddr(addr), 0, size);
    }

    used += size;
//    DEBUG("kernel: allocated %d bytes at %p (%dKB used)",
//        size, addr, used / 1024);

    return addr;
}


void *kmalloc(size_t size, int flags) {
    return from_paddr(alloc_pages(size, flags));
}


paddr_t v_allocated;
uptr_t valloc(struct vmspace *vms, size_t size) {
    // XXX
    uptr_t v = v_allocated;
    v_allocated += ROUND_UP(size, PAGE_SIZE);
    return v;
}

void kfree(UNUSED void *ptr) {
}

void add_vmarea(
    struct vmspace *vms,
    uptr_t address,
    uptr_t pager_offset,
    size_t length,
    int flags,
    paddr_t (*pager)(void *arg, uptr_t offset, size_t length),
    void *pager_arg
) {
    struct vmarea *area = kmalloc(sizeof(*area), KMALLOC_NORMAL);
    area->address = address;
    area->length = length;
    area->flags = flags;
    area->pager = pager;
    area->pager_offset = pager_offset;
    area->pager_arg = pager_arg;
    vmarea_list_append(&vms->vma, area);
}


void memory_create_vmspace(struct vmspace *vms) {

    vmarea_list_init(&vms->vma);
    arch_create_vmspace(&vms->arch);
}


void memory_destroy_vmspace(UNUSED struct vmspace *vms) {

    PANIC("%s: not yet implemented", __func__);
}

paddr_t zeroed_pager(void *arg, off_t offset, size_t length) {
    return alloc_pages(length, KMALLOC_NORMAL);
}


void handle_page_fault(uptr_t original_address, bool present, bool user, bool write, UNUSED bool exec) {
    uptr_t address = ROUND_DOWN(original_address, PAGE_SIZE);

    if (!user) {
        PANIC("#PF in kernel: %p", original_address);
    }

    if (present) {
        DEBUG("PF in a present page");
        goto invalid_access;
    }

    struct vmspace *vms = &CPUVAR->current->process->vms;
    for (struct vmarea *area = vms->vma; area != NULL; area = area->next) {
        if (area->address <= address && address < area->address + area->length) {
            int requested = 0;
            requested |= user ? PAGE_USER : 0;
            requested |= write ? PAGE_WRITABLE : 0;
            // TODO: NX-bit

            // Check the access right.
            if ((area->flags & requested) != requested) {
                continue;
            }

            // A valid page access. Fill and link the page.
            off_t offset = address - area->address + area->pager_offset;
            paddr_t paddr = area->pager(area->pager_arg, offset, PAGE_SIZE);
            if (!paddr) {
                INFO("page fault: pager error");
                thread_destroy_current();
            }

            INFO("filling %p -> %p %p    ", paddr, address, area->flags);
            arch_link_page(&vms->arch, address, paddr, 1, area->flags);
            return;
        }
    }

invalid_access:
    INFO("page fault: invalid page access %p", original_address);
    thread_destroy_current();
}


void memory_init(void) {
    allocated = 0x001000000;
    v_allocated = 0xa00000000;
}
