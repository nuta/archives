#include "_x86.h"
#include <hal.h>


static void x86_set_seg_desc (struct seg_desc *desc, paddr_t base, paddr_t limit,
                              uint8_t type, uint8_t limit2_mask) {

    desc->limit1  = limit & 0xffff;
    desc->limit2  = (uint8_t) ((limit >> 16) & 0x0f) | limit2_mask;
    desc->base1   = base & 0xffff;
    desc->base2   = (base >> 16) & 0xff;
    desc->base3   = (base >> 24) & 0xff;
    desc->type    = type;
    return;
}

static void x86_set_tss_desc (struct tss_desc *desc, paddr_t base) {

    desc->limit1   = GDT_TSS_LIMIT & 0xffff;
    desc->limit2   = (GDT_TSS_LIMIT >> 16) & 0x0f;
    desc->base1    = base & 0xffff;
    desc->base2    = (base >> 16) & 0xff;
    desc->base3    = (base >> 24) & 0xff;
    desc->type     = GDTTYPE_TSS;
    desc->base4    = (uint32_t) (base >> 32) & 0xffffffff;
    desc->reserved = 0;
    return;
}


void  x86_init_gdt (struct seg_desc *gdt, struct TSS *tss) {

    // NULL
    x86_set_seg_desc(&gdt[GDT_NULL], 0, 0, 0, 0);
    // 64-bit code segment (kernel)
    x86_set_seg_desc(&gdt[GDT_KERNEL_CODE], 0x0000000000000000, 0xffffffffffffffff,
                     GDTTYPE_KERNEL_CODE64, GDT_LIMIT2_MASK_CODE64);
    // 64-bit data segment (kernel)
    x86_set_seg_desc(&gdt[GDT_KERNEL_DATA], 0x0000000000000000, 0xffffffffffffffff,
                     GDTTYPE_KERNEL_DATA64, GDT_LIMIT2_MASK_DATA64);
    // 32-bit code segment (user)
    x86_set_seg_desc(&gdt[GDT_USER_CODE32], 0x0000000000000000, 0xffffffffffffffff,
                     GDTTYPE_USER_CODE32, GDT_LIMIT2_MASK_CODE32);
    // 64-bit code segment (user)
    x86_set_seg_desc(&gdt[GDT_USER_CODE], 0x0000000000000000, 0xffffffffffffffff,
                     GDTTYPE_USER_CODE64, GDT_LIMIT2_MASK_CODE64);
    // 64-bit data segment (user)
    x86_set_seg_desc(&gdt[GDT_USER_DATA], 0x0000000000000000, 0xffffffffffffffff,
                     GDTTYPE_USER_DATA64, GDT_LIMIT2_MASK_DATA64);
    // TSS
    x86_set_tss_desc((struct tss_desc *) &gdt[GDT_TSS], (paddr_t) tss);
}
