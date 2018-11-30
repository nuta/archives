#include "string.h"
#include "printk.h"
#include "types.h"
#include "thread.h"
#include "process.h"
#include "memory.h"

paddr_t allocated_pages_start;
size_t allocated_pages = 0;
static struct page *pages;

paddr_t alloc_pages(size_t num, UNUSED int flags) {
retry:
    for (size_t i = 0; i < phypages_num; i++) {
        size_t j;
        for (j = 0; i + j < phypages_num && j < num; j++) {
            if (pages[i + j].ref_count != 0) {
                break;
            }
        }

        if (j == num) {
            /* Found a large enough space. Mark pages as used. */
            for (j = 0; j < num; j++) {
                if (!atomic_compare_and_swap(&pages[i + j].ref_count, 0, 1)) {
                    /* Failed to mark as used. Deallocate pages and try again. */
                    for (size_t k = 0; k < j; k++) {
                        atomic_compare_and_swap(&pages[i + j].ref_count, 1, 0);
                    }

                    goto retry;
                }
            }

            allocated_pages += num;
            return allocated_pages_start + (i * PAGE_SIZE);
        }
    }

    PANIC("run out of memory");
}

void free_pages(paddr_t addr, size_t num) {
    size_t offset = addr - allocated_pages_start;
    if (offset % PAGE_SIZE != 0) {
        BUG("free_pages: addr (%p) is not aligned to PAGE_SIZE", addr);
        return;
    }

    size_t start = offset / PAGE_SIZE;
    for (size_t i = 0; i < num; i++) {
        atomic_fetch_and_sub(&pages[start + i].ref_count, 1);
    }

    allocated_pages -= num;
}


void *kmalloc(size_t size, int flags) {
    void *ptr = from_paddr(alloc_pages(LEN_TO_PAGE_NUM(size), flags));

    if (flags & KMALLOC_ZEROED) {
        memset(ptr, 0, size);
    }

    return ptr;
}


/* Allocates a *virtual* memory pages. Caller must link them with
   physical pages by arch_link_page(). */
uptr_t valloc(struct vmspace *vms, size_t num) {
retry:
    for (size_t i = 0; i < vms->dyn_pages_max; i++) {
        size_t j;
        for (j = 0; i + j < vms->dyn_pages_max && j < num; j++) {
            if (vms->dyn_pages[i + j] != 0) {
                /* The page is being used; try again from the beginning. */
                break;
            }
        }

        if (j == num) {
            /* Found a large enough space. Mark pages as used. */
            for (j = 0; j < num; j++) {
                if (!atomic_compare_and_swap(&vms->dyn_pages[i + j], 0, DYN_PAGE_USING)) {
                    /* Failed to mark as used. Deallocate pages and try again. */
                    for (size_t k = 0; k < j; k++) {
                        atomic_compare_and_swap(&vms->dyn_pages[i + j], DYN_PAGE_USING, 0);
                    }

                    goto retry;
                }
            }

            vms->dyn_pages[i + num - 1] = DYN_PAGE_END;
            return DYN_PAGE_BASE_ADDR + i * PAGE_SIZE;
        }
    }

    return 0;
}

void vfree(struct vmspace *vms, uptr_t vaddr) {
    size_t start = (vaddr - DYN_PAGE_BASE_ADDR) / PAGE_SIZE;

    for (size_t i = start; i < vms->dyn_pages_max; i++) {
        u8_t type = vms->dyn_pages[i];
        vms->dyn_pages[i] = DYN_PAGE_UNUSED;
        if (type == DYN_PAGE_END) {
            break;
        }
    }
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
    vms->dyn_pages_max = DEFAULT_DYN_PAGES_MAX;
    vms->dyn_pages = (u8_t *) kmalloc(DEFAULT_DYN_PAGES_MAX * sizeof(u8_t),
                                      KMALLOC_NORMAL | KMALLOC_ZEROED);
    vmarea_list_init(&vms->vma);
    arch_create_vmspace(&vms->arch);
}


void memory_destroy_vmspace(UNUSED struct vmspace *vms) {

    PANIC("%s: not yet implemented", __func__);
}

paddr_t zeroed_pager(void *arg, off_t offset, size_t length) {
    return alloc_pages(LEN_TO_PAGE_NUM(length), KMALLOC_NORMAL);
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

size_t get_allocated_pages(void) {
    return allocated_pages;
}

void memory_init(void) {
    pages = (struct page *) from_paddr(phypages_start);
    size_t phypages_size = sizeof(struct page) * phypages_num;
    memset(pages, 0, phypages_size);
    allocated_pages_start = ROUND_UP(phypages_start + phypages_size, PAGE_SIZE);
}
