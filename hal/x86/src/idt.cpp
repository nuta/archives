#include "_x86.h"
#include <hal.h>


void x86_set_int_desc (struct int_desc *desc, uint8_t ist, uint16_t seg, paddr_t offset) {

    desc->offset1  = offset & 0xffff;
    desc->seg      = seg;
    desc->ist      = ist;
    desc->info     = IDT_INT_HANDLER;
    desc->offset2  = (offset >> 16) & 0xffff;
    desc->offset3  = (offset >> 32) & 0xffffffff;
    desc->reserved = 0;
    return;
}


uint8_t x86_alloc_intr_vector (void) {
    struct int_desc *idt;
    uint8_t i;

    idt = (struct int_desc *) &CPUVAR->idt;

    for (i=32; i <= 250; i++) {
        if (((paddr_t) idt[i].offset3 << 32 | idt[i].offset2 << 16 | idt[i].offset1) ==
                (paddr_t) x86_asm_int_handler_unregistered)
            break;
    }

    return ((i == 250)? 0 : i);
}


void x86_init_idt (struct int_desc *idt) {

    // exceptions
    x86_set_int_desc((struct int_desc *) &idt[0], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler0);
    x86_set_int_desc((struct int_desc *) &idt[1], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler1);
    x86_set_int_desc((struct int_desc *) &idt[2], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler2);
    x86_set_int_desc((struct int_desc *) &idt[3], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler3);
    x86_set_int_desc((struct int_desc *) &idt[4], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler4);
    x86_set_int_desc((struct int_desc *) &idt[5], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler5);
    x86_set_int_desc((struct int_desc *) &idt[6], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler6);
    x86_set_int_desc((struct int_desc *) &idt[7], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler7);
    x86_set_int_desc((struct int_desc *) &idt[8], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler8);
    x86_set_int_desc((struct int_desc *) &idt[9], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler9);
    x86_set_int_desc((struct int_desc *) &idt[10], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler10);
    x86_set_int_desc((struct int_desc *) &idt[11], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler11);
    x86_set_int_desc((struct int_desc *) &idt[12], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler12);
    x86_set_int_desc((struct int_desc *) &idt[13], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler13);
    x86_set_int_desc((struct int_desc *) &idt[14], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler14);
    x86_set_int_desc((struct int_desc *) &idt[15], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler15);
    x86_set_int_desc((struct int_desc *) &idt[16], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler16);
    x86_set_int_desc((struct int_desc *) &idt[17], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler17);
    x86_set_int_desc((struct int_desc *) &idt[18], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler18);
    x86_set_int_desc((struct int_desc *) &idt[19], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler19);
    x86_set_int_desc((struct int_desc *) &idt[20], INT_HANDLER_IST,
                     KERNEL_CODE64_SEG, (paddr_t) &x86_asm_exp_handler20);

    // 21-31: reserved

    // 32-: user defined
    for (int i=32; i <= 255; i++) {
        x86_set_int_desc((struct int_desc *) &idt[i], INT_HANDLER_IST,
                         KERNEL_CODE64_SEG, (paddr_t) &x86_asm_int_handler_unregistered);
    }
}
