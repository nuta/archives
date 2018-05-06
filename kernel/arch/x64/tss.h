#ifndef __X64_TSS_H__
#define __X64_TSS_H__

struct tss {
    u32_t  reserved0;
    u64_t  rsp0;
    u64_t  rsp1;
    u64_t  rsp2;
    u64_t  reserved1;
    u64_t  ist[7];
    u64_t  reserved2;
    u16_t  reserved3;
    u16_t  iomap;
} PACKED;

#define INTR_STACK_SIZE 4096
#define INTR_HANDLER_IST 1

void x64_init_tss(void);

#endif
