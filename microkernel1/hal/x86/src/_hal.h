#ifndef ___HAL_H__
#define ___HAL_H__

#include <resea.h>

// memory
#define PAGE_SIZE  4096

struct hal_vm_space{
  mutex_t    lock;
  uint64_t*  pml4;
};

// threading
struct x86_thread_regs{
  uint64_t  rax;
  uint64_t  rbx;
  uint64_t  rcx;
  uint64_t  rdx;
  uint64_t  rsi;
  uint64_t  rdi;
  uint64_t  rbp;
  uint64_t  r8;
  uint64_t  r9;
  uint64_t  r10;
  uint64_t  r11;
  uint64_t  r12;
  uint64_t  r13;
  uint64_t  r14;
  uint64_t  r15;

  uint64_t  rip;
  uint64_t  cs;
  uint64_t  rflags;
  uint64_t  rsp;
  uint64_t  ss;
} PACKED;

struct hal_thread{
  bool is_kernel;
  struct x86_thread_regs regs;
  void* fregs; // FXSAVE area, MUST be aligned to 16-byte boundary
};


// CPU
struct int_desc {
    uint16_t offset1;
    uint16_t seg;
    uint8_t  ist;
    uint8_t  info;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
} PACKED;

#define IDT_DESC_NUM    256
#define IDT_LENGTH      ((IDT_DESC_NUM * sizeof(struct int_desc)) + 1)
#define IDT_INT_HANDLER 0x8e

struct IDTR {
    uint16_t length;
    uint64_t address;
} PACKED;
#define GDT_NULL        0
#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE32 3
#define GDT_USER_DATA   4
#define GDT_USER_CODE   5
#define GDT_TSS         6 // Note: a TSS descriptor is twice as large as
                          //       a code segment descriptor
#define GDT_DESC_NUM    8

#define KERNEL_NULL_SEG    0
#define KERNEL_CODE64_SEG  (GDT_KERNEL_CODE * 8)
#define KERNEL_DATA64_SEG  (GDT_KERNEL_DATA * 8)
#define USER_DATA64_SEG    (GDT_USER_DATA   * 8)
#define USER_CODE32_SEG    (GDT_USER_CODE32 * 8)
#define USER_CODE64_SEG    (GDT_USER_CODE   * 8)
#define GDT_TSS_SEG        (GDT_TSS         * 8)

#define GDT_LENGTH                ((sizeof(struct seg_desc) * GDT_DESC_NUM) - 1)
#define GDTTYPE_KERNEL_CODE64     0x9a /* FIXME: prefixes (-64 and -32) are not */
#define GDTTYPE_KERNEL_DATA64     0x92 /* necessary in GDTTYPE */
#define GDTTYPE_USER_CODE32       0xfa
#define GDTTYPE_USER_CODE64       0xfa
#define GDTTYPE_USER_DATA64       0xf2
#define GDTTYPE_TSS               0x89
#define GDT_TSS_LIMIT             (sizeof(struct TSS) - 1)
#define GDT_LIMIT2_MASK_CODE64    0xa0
#define GDT_LIMIT2_MASK_CODE32    0xc0
#define GDT_LIMIT2_MASK_DATA64    0x80

struct seg_desc {
    uint16_t limit1;
    uint16_t base1;
    uint8_t  base2;
    uint8_t  type;
    uint8_t  limit2;
    uint8_t  base3;
} PACKED;

struct tss_desc {
    uint16_t limit1;
    uint16_t base1;
    uint8_t  base2;
    uint8_t  type;
    uint8_t  limit2;
    uint8_t  base3;
    uint32_t base4;
    uint32_t reserved;
} PACKED;

struct GDTR{
    uint16_t length;
    uint64_t address;
} PACKED;

struct TSS {
    uint32_t  reserved0;
    uint64_t  rsp0;
    uint64_t  rsp1;
    uint64_t  rsp2;
    uint64_t  reserved1;
    uint64_t  ist[7];
    uint64_t  reserved2;
    uint16_t  reserved3;
    uint16_t  iomap;
} PACKED;
struct x86_cpuvar{
  struct seg_desc gdt[GDT_DESC_NUM];
  struct int_desc idt[IDT_DESC_NUM];
  struct GDTR gdtr;
  struct IDTR idtr;
  struct TSS tss;
  uint64_t intr_stack_addr;
  ident_t current_thread;
};

#define CPU_MAX_NUM 32
extern struct x86_cpuvar x86_cpuvars[CPU_MAX_NUM];
#define CPUVAR ((struct x86_cpuvar *) &x86_cpuvars[hal_get_cpuid()])

#endif
