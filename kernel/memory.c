#include "string.h"
#include "printk.h"
#include "types.h"
#include "thread.h"
#include "process.h"
#include "memory.h"

paddr_t allocated_pages_start;
static struct page *pages;

paddr_t alloc_pages(size_t size, UNUSED int flags) {
    size_t num, start;

retry:
    num = ROUND_UP(size, PAGE_SIZE) / PAGE_SIZE;
    start = 0;
    for (size_t i = 0; i < phypages_num; i++) {
        size_t j;
        for (j = 0; i + j < phypages_num && j < num; j++) {
            if (pages[i + j].ref_count != 0) {
                break;
            }
        }

        if (j == num) {
            /* Found a large enough space. Mark pages as used. */
            for (j = start; j < start + num; j++) {
                if (!atomic_compare_and_swap(&pages[i + j].ref_count, 0, 1)) {
                    /* Failed to mark as used. Deallocate pages and try again. */
                    for (size_t k = start; k < j; k++) {
                        atomic_compare_and_swap(&pages[i + j].ref_count, 1, 0);
                    }

                    goto retry;
                }
            }

            return allocated_pages_start + (i * PAGE_SIZE);
        }
    }

    PANIC("run out of memory");
}


void *kmalloc(size_t size, int flags) {
    void *ptr = from_paddr(alloc_pages(size, flags));

    if (flags & KMALLOC_ZEROED) {
        memset(ptr, 0, size);
    }

    return ptr;
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
    off_t pager_offset,
    size_t length,
    int flags,
    pager_t *pager,
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

            DEBUG("filling %p -> %p %p    ", paddr, address, area->flags);
            arch_link_page(&vms->arch, address, paddr, 1, area->flags);
            return;
        }
    }

invalid_access:
    INFO("page fault: invalid page access %p (#%d.%d)",
        original_address, CPUVAR->current->process->pid, CPUVAR->current->tid);
    thread_destroy_current();
}

void memory_init(void) {
    pages = (struct page *) from_paddr(phypages_start);
    size_t phypages_size = sizeof(struct page) * phypages_num;
    memset(pages, 0, phypages_size);
    allocated_pages_start = ROUND_UP(phypages_start + phypages_size, PAGE_SIZE);
    v_allocated = 0xa00000000;
}
