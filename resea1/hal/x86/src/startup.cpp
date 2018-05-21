#include <hal.h>
#include "x86.h"


namespace {
    struct hal_pmmap bsp_pmmap[16];
    struct hal_vmmap bsp_vmmap[16];
}

void start_apps(void);
extern "C" void kernel_startup(void);
void x86_init_localapic_timer(void);


struct hal_pmmap *hal_get_pmmaps(void) {

    return (struct hal_pmmap *) &bsp_pmmap;
}


struct  hal_vmmap *hal_get_vmmaps(void) {

    return (struct hal_vmmap *) &bsp_vmmap;
}


//
//  Initializes GDT, IDT, TSS, and other x86-related stuffs and
//  calls the kernek. This function MUST be called only from BSP, only once.
//
void x86_init(void *binfo) {
    uint32_t *p;
    uint32_t type, size;
    uint64_t q;

    x86_init_print();
    x86_disable_pic(); // disable PIC; use IO APIC instead
    x86_init_localapic();

    // look for memory map passed by GRUB2
    p = (uint32_t *) ((paddr_t) binfo + 8);
    for (;;) {
        type = *p & 0xffffffff;
        size = *(uint32_t *) ((paddr_t) p + 4) & 0xffffffff;

        // is a memory map tag?
        if (type == 0 || type == 6)
            break;

        // go to the next tag
        p = (uint32_t *) ((uint64_t) p + size);
        q = (uint64_t) p;

        // all tags are aligned in 8-byte boundaries
        p = (uint32_t *) (((q & 7) == 0)? q : (q + 7) & 0xfffffffffffffff8);
    }

    // memory map not found
    if (type != 6) {
        PANIC("memory map from GRUB2 not found");
    }

    // convert the GRUB2 memory map to hal_pmmap
    uint32_t i, j;
    uint32_t num = (size - 16) / sizeof(struct grub_mmap_entry);
    struct grub_mmap_entry *m = (struct grub_mmap_entry *) ((paddr_t) p + 16);

    for (i=0, j=0; i < 15 && j < num; j++) {

        if ((m->type & 0xffffffff) == 1 && m->base > 0) {
            DEBUG("found a available RAM space: addr=%p, size=%dMiB",
                  m->base, m->length / 1024 / 1024);

            if (m->base < FREE_MEMORY_BASE_ADDR) {
                // 0x0 - FREE_MEMORY_BASE_ADDR are reserved by kernel (the kernel
                // image and page table exists)
                bsp_pmmap[i].addr = FREE_MEMORY_BASE_ADDR;
                bsp_pmmap[i].size = m->length - (FREE_MEMORY_BASE_ADDR - m->base);
            } else {
               bsp_pmmap[i].addr = m->base;
               bsp_pmmap[i].size = m->length;
            }

            i++;
        }

        m = (struct grub_mmap_entry *) ((paddr_t) m + 24);
    }

    bsp_pmmap[i].addr = 0;
    bsp_pmmap[i].size = 0;

    // virtual memory map
    bsp_vmmap[0].type = VMMAP_USER;
    bsp_vmmap[0].addr = 0x0000000000000000;
    bsp_vmmap[0].size = KERNEL_BASE_ADDR - DYNAMIC_PAGETABLE_SIZE;

    bsp_vmmap[1].type = VMMAP_DYNAMIC;
    bsp_vmmap[1].addr = KERNEL_BASE_ADDR - DYNAMIC_PAGETABLE_SIZE;
    bsp_vmmap[1].size = DYNAMIC_PAGETABLE_SIZE;

    bsp_vmmap[2].type = VMMAP_KERNEL;
    bsp_vmmap[2].addr = KERNEL_BASE_ADDR;
    bsp_vmmap[2].size = 0xffffffffffffffff - KERNEL_BASE_ADDR + 1;

    bsp_vmmap[3].addr = 0;
    bsp_vmmap[3].size = 0;


    // Initialize GDT
    x86_init_gdt((struct seg_desc *) &CPUVAR->gdt, &CPUVAR->tss);
    CPUVAR->gdtr.length  = GDT_LENGTH;
    CPUVAR->gdtr.address = (uint64_t) &CPUVAR->gdt;
    x86_asm_lgdt((uint64_t) &CPUVAR->gdtr);

    // Initialize IDT
    x86_init_idt((struct int_desc *) &CPUVAR->idt);
    CPUVAR->idtr.length  = IDT_LENGTH;
    CPUVAR->idtr.address = (uint64_t) &CPUVAR->idt;
    x86_asm_lidt((uint64_t) &CPUVAR->idtr);

    // Initialize I/O APIC, etc.
    x86_init_smp();

    // Enable system call instructions
    x86_asm_init_syscall((void (*)) x86_asm_syscall_handler,
                         KERNEL_CODE64_SEG,
                         KERNEL_NULL_SEG);

    // Initialize virtual memory
    x86_init_vm();

    // start the kernel
    kernel_startup();

    x86_init_tss(&CPUVAR->tss);
    x86_asm_ltr(GDT_TSS_SEG);
    x86_init_localapic_timer();

    // start applications
    start_apps();
}

