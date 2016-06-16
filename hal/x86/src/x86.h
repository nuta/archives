#ifndef __X86_H__
#define __X86_H__

#include <resea.h>
#include "_x86.h"
#include "common.h"

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


/*
 * asm
 */

extern "C" {
uint8_t x86_asm_in8(uint16_t port);
void x86_asm_out8(uint16_t port, uint8_t data);
uint16_t x86_asm_in16(uint16_t port);
void x86_asm_out16(uint16_t port, uint16_t data);
uint32_t x86_asm_in32(uint16_t port);
void x86_asm_out32(uint16_t port, uint32_t data);
void x86_asm_invalidate_tlb(paddr_t paddr);
void x86_asm_lidt(uint64_t idtr);
void x86_asm_lgdt(uint64_t gdtr);
void x86_asm_ltr(uint16_t tr);
uint64_t x86_asm_rdmsr(uint32_t reg);
void x86_asm_wrmsr(uint32_t reg, uint64_t data);
uint64_t x86_asm_read_cr2(void);
bool x86_asm_interrupt_enabled(void);
void x86_asm_cli(void);
void x86_asm_sti(void);
void x86_asm_hlt(void);
void x86_asm_stihlt(void);
NORETURN void x86_asm_resume_thread(bool is_kernel, struct x86_thread_regs *regs);
void x86_asm_init_syscall(void(*handler), uint16_t cs0, uint16_t cs3);
void x86_unblocked_thread_entry(uint64_t rsp);
void x86_asm_set_cr3(uint64_t cr3);
void x86_asm_fxsave(void *xsave_area);
void x86_asm_fxrstor(void *xsave_area);
}

/*
 * TSS
 */

#define INT_HANDLER_IST  1

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

void x86_init_tss (struct TSS *tss);


/*
 * GDT
 */

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

void x86_init_gdt(struct seg_desc *gdt, struct TSS *tss);


/*
 * IDT
 */

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

void x86_init_idt(struct int_desc *idt);
void x86_set_int_desc(struct int_desc *desc, uint8_t ist, uint16_t seg, paddr_t offset);
uint8_t x86_alloc_intr_vector(void);


/*
 * GRUB
 */

struct grub_mmap_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} PACKED;


/*
 *  Exceptions
 */
extern "C" {
void x86_exp_handler0 (uint64_t rip);
void x86_exp_handler1 (uint64_t rip);
void x86_exp_handler2 (uint64_t rip);
void x86_exp_handler3 (uint64_t rip);
void x86_exp_handler4 (uint64_t rip);
void x86_exp_handler5 (uint64_t rip);
void x86_exp_handler6 (uint64_t rip);
void x86_exp_handler7 (uint64_t rip);
void x86_exp_handler8 (uint64_t rip, uint64_t errcode);
void x86_exp_handler9 (uint64_t rip);
void x86_exp_handler10 (uint64_t rip, uint64_t errcode);
void x86_exp_handler11 (uint64_t rip, uint64_t errcode);
void x86_exp_handler12 (uint64_t rip, uint64_t errcode);
void x86_exp_handler13 (uint64_t rip, uint64_t errcode);
void x86_exp_handler14 (uint64_t rip, uint64_t errcode);
void x86_exp_handler15 (uint64_t rip);
void x86_exp_handler16 (uint64_t rip);
void x86_exp_handler17 (uint64_t rip, uint64_t errcode);
void x86_exp_handler18 (uint64_t rip);
void x86_exp_handler19 (uint64_t rip);
void x86_exp_handler20 (uint64_t rip);
}

/*
 * handler
 */

/* declared in handler.S */
extern "C" {
void x86_asm_exp_handler0(void);
void x86_asm_exp_handler1(void);
void x86_asm_exp_handler2(void);
void x86_asm_exp_handler3(void);
void x86_asm_exp_handler4(void);
void x86_asm_exp_handler5(void);
void x86_asm_exp_handler6(void);
void x86_asm_exp_handler7(void);
void x86_asm_exp_handler8(void);
void x86_asm_exp_handler9(void);
void x86_asm_exp_handler10(void);
void x86_asm_exp_handler11(void);
void x86_asm_exp_handler12(void);
void x86_asm_exp_handler13(void);
void x86_asm_exp_handler14(void);
void x86_asm_exp_handler15(void);
void x86_asm_exp_handler16(void);
void x86_asm_exp_handler17(void);
void x86_asm_exp_handler18(void);
void x86_asm_exp_handler19(void);
void x86_asm_exp_handler20(void);

void x86_asm_int_handler32(void);
void x86_asm_int_handler33(void);
void x86_asm_int_handler34(void);

void x86_asm_int_handler_unregistered(void);
void x86_asm_syscall_handler(void);
}


/*
 * Interrupt
 */

#define INT_HANDLER_STACK_SIZE  8192
uint8_t alloc_intr_vector(void);


/*
 * IO APIC
 */

#define IOAPIC_IOREGSEL_OFFSET  0x00
#define IOAPIC_IOWIN_OFFSET     0x10

#define IOAPIC_REG_IOAPICVER  0x01
#define IOAPIC_REG_NTH_IOREDTBL_LOW(n)  (0x10 + (n * 2))
#define IOAPIC_REG_NTH_IOREDTBL_HIGH(n) (0x10 + (n * 2) + 1)

void x86_init_ioapic(paddr_t mmio);
void x86_ioapic_enable_irq(uint8_t vector, uint8_t irq);


/*
 * Local APIC
 */

#define LOCALAPIC_REG_ID            0xfee00020
#define LOCALAPIC_REG_VERSION       0xfee00030
#define LOCALAPIC_REG_TPR           0xfee00080
#define LOCALAPIC_REG_EOI           0xfee000b0
#define LOCALAPIC_REG_LOGICAL_DEST  0xfee000d0
#define LOCALAPIC_REG_DEST_FORMAT   0xfee000e0
#define LOCALAPIC_REG_SPURIOUS_INT  0xfee000f0
#define LOCALAPIC_REG_ICR_LOW       0xfee00300
#define LOCALAPIC_REG_ICR_HIGH      0xfee00310
#define LOCALAPIC_REG_LVT_TIMER     0xfee00320
#define LOCALAPIC_REG_LINT0         0xfee00350
#define LOCALAPIC_REG_LINT1         0xfee00360
#define LOCALAPIC_REG_LVT_ERROR     0xfee00370
#define LOCALAPIC_REG_TIMER_INITCNT 0xfee00380
#define LOCALAPIC_REG_TIMER_CURRENT 0xfee00390
#define LOCALAPIC_REG_TIMER_DIV     0xfee003e0

#define MSR_IA32_APIC_BASE  0x1b

#define IPI_TYPE_FIXED      0 //
#define IPI_TYPE_NMI        4 // Non-Maskable Interrupt
#define IPI_TYPE_INIT       5 // initialize AP
#define IPI_TYPE_STARTUP    6 // startup AP
#define IPI_DEST_ONE        0 // send to a processor specified in `dest`
#define IPI_DEST_SELF       1 // sent to itself
#define IPI_DEST_ALL        2 // send to all processors including itself
#define IPI_DEST_ALL_EXSELF 3 // send to all processors excluding itself

void x86_init_localapic(void);
extern "C" void x86_ack_interrupt(void);

extern uint8_t x86_timer_vector;

/*
 * PIC
 */

void x86_disable_pic(void);

/*
 *  FPU
 */
#define XSAVE_AREA_SIZE  512

/*
 * Print
 */

#define VRAM_ADDR  (PHY(0xb8000))
#define SCREEN_X_MAX 80
#define SCREEN_Y_MAX 25

#define IOPORT_SERIALPORT    0x03f8
#define IOPORT_SERIAL_DATA   (IOPORT_SERIALPORT + 0)
#define IOPORT_SERIAL_STATUS (IOPORT_SERIALPORT + 5)

#define VRAM_CHAR_AT(y,x)  *((char *) VRAM_ADDR + ((y)*SCREEN_X_MAX*2) + ((x)*2))
#define VRAM_COLOR_AT(y,x) *((char *) VRAM_ADDR + ((y)*SCREEN_X_MAX*2) + ((x)*2) + 1)

void x86_init_print(void);


/*
 * SMP
 */

#define MP_BASETABLE_PROCESSOR_ENTRY        0
#define MP_BASETABLE_BUS_ENTRY              1
#define MP_BASETABLE_IOAPIC_ENTRY           2
#define MP_BASETABLE_IOINT_ASSIGN_ENTRY     3
#define MP_BASETABLE_LOCALINT_ASSIGN_ENTRY  4

struct mp_float_ptr {
    uint8_t  signature[4];  // "_MP_"
    uint32_t mptable_header_addr;
    uint8_t  length;
    uint8_t  spec_rev;
    uint8_t  checksum;
    uint8_t  info1;
    uint8_t  info2;
    uint8_t  info3[3];
} PACKED;

struct mp_table_header {
    uint8_t  signature[4]; // "PCMP"
    uint16_t base_table_length;
    uint8_t  spec_rev;
    uint8_t  checksum;
    uint8_t  oem_id[8];
    uint8_t  product_id[12];
    uint32_t oem_table_pointer;
    uint16_t oem_table_size;
    uint16_t entry_count;
    uint32_t memmaped_localapic_addr;
    uint16_t extended_table_length;
    uint8_t  extended_table_checksum;
    uint8_t  reserved;
} PACKED;

struct mp_processor_entry {
    uint8_t  type;          // 0
    uint8_t  localapic_id;
    uint8_t  localapic_ver;
    uint8_t  cpu_flags;
    uint32_t cpu_signature;
    uint32_t feature_flags;
    uint32_t reserved1;
    uint32_t reserved2;
} PACKED;

struct mp_bus_entry{
    uint8_t  type;          // 1
    uint8_t  id;
    uint8_t  type_str[6];
} PACKED;

struct mp_ioapic_entry {
    uint8_t  type;         // 2
    uint8_t  ioapic_id;
    uint8_t  ioapic_ver;
    uint8_t  ioapic_flags;
    uint32_t memmaped_ioapic_addr;
} PACKED;

struct mp_ioint_assign_entry {
    uint8_t  type;         // 3
    uint8_t  int_type;
    uint16_t int_flags;
    uint8_t  src_bus_id;
    uint8_t  src_bus_irq;
    uint8_t  dest_ioapic_id;
    uint8_t  dest_ioapic_intin;
} PACKED;

struct mp_localint_assign_entry {
    uint8_t  type;         // 4
    uint8_t  int_type;
    uint16_t int_flags;
    uint8_t  src_bus_id;
    uint8_t  src_bus_irq;
    uint8_t  dest_localapic_id;
    uint8_t  dest_localapic_intin;
} PACKED;

void x86_init_smp(void);


/*
 * Memory
 */

/* XXX */
#define PHY(x) (KERNEL_BASE_ADDR + ((uint64_t) (x)))

/* the number of entries of PML4, PDPT, PD and PT are same: 512 */
#define PAGE_ENTRY_NUM 512
#define DYNAMIC_PAGETABLE_SIZE (512 * 1024 * 1024) // 512MiB

void x86_init_vm(void);

/*
 *  init
 */
void x86_init(void *binfo);

#endif
