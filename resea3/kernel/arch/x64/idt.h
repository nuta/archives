#ifndef __X64_IDT_H__
#define __X64_IDT_H__

#include <kernel/types.h>

#define IDT_LENGTH      ((IDT_DESC_NUM * sizeof(struct intr_desc)) + 1)
#define IDT_INT_HANDLER 0x8e

void x64_init_idt(void);
void x64_set_intr_desc(struct intr_desc *desc, u8_t ist, u16_t seg, paddr_t offset);

#endif
