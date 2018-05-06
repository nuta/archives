#ifndef __X64_IDT_H__
#define __X64_IDT_H__

#include <kernel/types.h>

#define IDT_DESC_NUM    256
#define IDT_LENGTH      ((IDT_DESC_NUM * sizeof(struct intr_desc)) + 1)
#define IDT_INT_HANDLER 0x8e

struct intr_desc {
    u16_t offset1;
    u16_t seg;
    u8_t  ist;
    u8_t  info;
    u16_t offset2;
    u32_t offset3;
    u32_t reserved;
} PACKED;

struct idtr {
    u16_t length;
    u64_t address;
} PACKED;

void x64_init_idt(void);
void x64_set_intr_desc(struct intr_desc *desc, u8_t ist, u16_t seg, paddr_t offset);

#endif
