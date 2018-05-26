#ifndef __X64_TSS_H__
#define __X64_TSS_H__

#define INTR_STACK_SIZE 4096
#define INTR_HANDLER_IST 1

void x64_init_tss(void);

#endif
