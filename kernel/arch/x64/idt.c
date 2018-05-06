#include <kernel/string.h>
#include "cpu.h"
#include "asm.h"
#include "idt.h"
#include "tss.h"
#include "handler.h"

void x64_set_intr_desc(struct intr_desc *desc, u8_t ist, u16_t seg, uptr_t offset) {

    desc->offset1  = offset & 0xffff;
    desc->seg      = seg;
    desc->ist      = ist;
    desc->info     = IDT_INT_HANDLER;
    desc->offset2  = (offset >> 16) & 0xffff;
    desc->offset3  = (offset >> 32) & 0xffffffff;
    desc->reserved = 0;
}


#define SET_EXP_DESC(n) set_kernel_intr_desc(&idt[n], (uptr_t) x64_exp_handler##n)
#define SET_IRQ_DESC(n) set_kernel_intr_desc(&idt[n], (uptr_t) x64_irq_handler##n)
static void set_kernel_intr_desc(struct intr_desc *desc, uptr_t offset) {

    desc->offset1  = offset & 0xffff;
    desc->seg      = KERNEL_CODE64_SEG;
    desc->ist      = INTR_HANDLER_IST;
    desc->info     = IDT_INT_HANDLER;
    desc->offset2  = (offset >> 16) & 0xffff;
    desc->offset3  = (offset >> 32) & 0xffffffff;
    desc->reserved = 0;
}


void x64_init_idt(void) {
    struct intr_desc *idt = (struct intr_desc *) &CPUVAR->idt;

    memset(&CPUVAR->idt, 0, sizeof(CPUVAR->idt));
    memset(&CPUVAR->idtr, 0, sizeof(CPUVAR->idtr));

    // expeptions
    SET_EXP_DESC(0);
    SET_EXP_DESC(1);
    SET_EXP_DESC(2);
    SET_EXP_DESC(3);
    SET_EXP_DESC(4);
    SET_EXP_DESC(5);
    SET_EXP_DESC(6);
    SET_EXP_DESC(7);
    SET_EXP_DESC(8);
    SET_EXP_DESC(9);
    SET_EXP_DESC(10);
    SET_EXP_DESC(11);
    SET_EXP_DESC(12);
    SET_EXP_DESC(13);
    SET_EXP_DESC(14);
    SET_EXP_DESC(15);
    SET_EXP_DESC(16);
    SET_EXP_DESC(17);
    SET_EXP_DESC(18);
    SET_EXP_DESC(19);
    SET_EXP_DESC(20);

    // Interrupts
    SET_IRQ_DESC(0x20);
    SET_IRQ_DESC(0x21);
    SET_IRQ_DESC(0x22);
    SET_IRQ_DESC(0x23);
    SET_IRQ_DESC(0x24);
    SET_IRQ_DESC(0x25);
    SET_IRQ_DESC(0x26);
    SET_IRQ_DESC(0x27);
    SET_IRQ_DESC(0x28);
    SET_IRQ_DESC(0x29);
    SET_IRQ_DESC(0x2a);
    SET_IRQ_DESC(0x2b);
    SET_IRQ_DESC(0x2c);
    SET_IRQ_DESC(0x2d);
    SET_IRQ_DESC(0x2e);
    SET_IRQ_DESC(0x2f);
    SET_IRQ_DESC(0x30);
    SET_IRQ_DESC(0x31);
    SET_IRQ_DESC(0x32);
    SET_IRQ_DESC(0x33);
    SET_IRQ_DESC(0x34);
    SET_IRQ_DESC(0x35);
    SET_IRQ_DESC(0x36);
    SET_IRQ_DESC(0x37);
    SET_IRQ_DESC(0x38);
    SET_IRQ_DESC(0x39);
    SET_IRQ_DESC(0x3a);
    SET_IRQ_DESC(0x3b);
    SET_IRQ_DESC(0x3c);
    SET_IRQ_DESC(0x3d);
    SET_IRQ_DESC(0x3e);
    SET_IRQ_DESC(0x3f);

    // To reduce the code size, we don't use all IDT entries for now.
    for (int i=0x40; i <= 0xff; i++) {
        x64_set_intr_desc(&idt[i], INTR_HANDLER_IST, KERNEL_CODE64_SEG, (uptr_t) x64_unknown_irq_handler);
    }

    // Update GDTR
    CPUVAR->idtr.length = IDT_LENGTH;
    CPUVAR->idtr.address = (uptr_t) idt;
    asm_lidt((uptr_t) &CPUVAR->idtr);
}
