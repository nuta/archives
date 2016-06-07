#include "_x86.h"
#include "x86.h"
#include <hal.h>
#include <resea.h>
#include <resea/cpp/memory.h>


static void set_page_attrs (uint64_t *page, page_attrs_t attrs) {
    uint64_t attr;
    bool user, writable, present;

    user       = attrs & PAGE_USER;
    writable   = attrs & PAGE_WRITABLE;
    present    = attrs & PAGE_PRESENT;

    attr  = 0;
    attr |= !!present;
    attr |= !!writable << 1;
    attr |= !!user     << 2;
// FIXME: I suppose that we must enable NX bit feature before
//    attr |= ((uint64_t) !!executable) << 63;

    *page |= attr;
}


static paddr_t lookup_page_entry (struct vm_space *vms, uintptr_t v, bool allocate,
                                page_attrs_t attrs, uint64_t **table, int *index) {
    int idx;
    uint64_t *t;

    t = (uint64_t *) hal_paddr_to_vaddr((paddr_t) vms->hal.pml4);

    for (int i=4; i > 1; i--) {
        idx = (v >> (((i-1) * 9) + 12))  & 0x1ff;

        if (!t[idx]) {
            /* the PDPT, PD or PT is not allocated so allocate it */
            if (allocate) {
                uintptr_t addr;
                paddr_t paddr;

                call_hal_callback(HAL_CALLBACK_ALLOCATE_MEMORY,
                    0, sizeof(uint64_t) * PAGE_ENTRY_NUM,
                    MEMORY_ALLOC_PAGE_ALIGNED | MEMORY_ALLOC_CONTINUOUS,
                    &addr, &paddr);

                t[idx] = paddr;
                set_page_attrs(&t[idx], attrs | PAGE_PRESENT);
            } else {
                BUG("the page does not exist");
                *table = nullptr;
                return 0;
            }
        }

        /* go into the next level paging table */
        t = (uint64_t *) hal_paddr_to_vaddr((uint64_t) t[idx] & 0x7ffffffffffff000);
    }

    /* t is now a pointer to the PT */
    idx = (v >> 12) & 0x1ff; // idx in PT

    if (table)
        *table = t;
    if (index)
        *index = idx;

    return (t[idx] & ~(0xfff));
}


void x86_init_vm(void) {

}


void hal_create_vm_space(struct vm_space *vms) {

    init_mutex(&vms->hal.lock, MUTEX_UNLOCKED);

    paddr_t paddr;
    call_hal_callback(HAL_CALLBACK_ALLOCATE_MEMORY,
        0, // paddr
        sizeof(uint64_t) * PAGE_ENTRY_NUM, // size
        MEMORY_ALLOC_PAGE_ALIGNED,
        &vms->hal.pml4, &paddr);

    /*
     *  Add page entries for the kernel space: reuse PDPT and PT which
     *  have already been created in start.S
     */
    vms->hal.pml4[(KERNEL_BASE_ADDR >> 39) & 0x1ff] = (PGTABLE_BASE_ADDR + 0x1000) | 0x3;
}


void hal_remove_vm_space(struct vm_space *vms) {

    /* TODO
      release_memory_block(vms->hal.pml4);
    */
}


void hal_switch_vm_space(struct hal_vm_space *vms) {
    uint64_t cr3;

    /* Note: CR3 stores *physical* address */
    cr3 = ((uint64_t) vms->pml4) & (~KERNEL_BASE_ADDR);
    x86_asm_set_cr3(cr3);
}


void hal_link_page(struct vm_space *vms, uintptr_t v, paddr_t p, size_t n,
                   page_attrs_t attrs) {
    int idx;
    uint64_t *table;

    /* pages for the kernel are already mapped */
    if (v > KERNEL_BASE_ADDR)
        return;

    lookup_page_entry(vms, v, true, attrs, &table, &idx);

    while(n > 0 && idx < PAGE_ENTRY_NUM) {
        table[idx] = p;
        set_page_attrs(&table[idx], attrs);
        x86_asm_invalidate_tlb(p);
        n--;
        idx++;
        p += PAGE_SIZE;
    }

    /* pages which belongs to the different PT */
    if (n > 0)
        hal_link_page(vms, v, p, n, attrs);
}


page_attrs_t hal_get_page_attribute(struct vm_space *vms, uintptr_t v) {
    int idx;
    page_attrs_t attrs;
    uint64_t *table;

    lookup_page_entry(vms, v, false, 0, &table, &idx);

    if (!table)
        PANIC("failed to get page attribute");

    attrs  = 0;
    attrs |= (((table[idx] >> 0)  & 1)? PAGE_PRESENT    : 0);
    attrs |= (((table[idx] >> 1)  & 1)? PAGE_READABLE   : 0);
    attrs |= (((table[idx] >> 2)  & 1)? PAGE_WRITABLE   : 0);
    attrs |= (((table[idx] >> 63) & 1)? PAGE_EXECUTABLE : 0);

    return attrs;
}


void hal_set_page_attribute(struct vm_space *vms, uintptr_t v, size_t n, page_attrs_t attrs) {

    hal_link_page(vms, v, 0, n, attrs);
}


paddr_t hal_vaddr_to_paddr(struct vm_space *vms, uintptr_t v) {

    if (!vms)
        BUG("`vms` should not be nullptr");

    if (v >= KERNEL_BASE_ADDR)
        return (v & ~(KERNEL_BASE_ADDR));

    return lookup_page_entry(vms, v, false, 0, nullptr, nullptr);
}


uintptr_t hal_paddr_to_vaddr(paddr_t v) {

    return (v | KERNEL_BASE_ADDR);
}
