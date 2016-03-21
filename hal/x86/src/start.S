#include "x86_asm.h"


#define MULTIBOOT_MAGIC    0xe85250d6
#define MULTIBOOT_ARCH     0 // i386
#define MULTIBOOT_LENGTH   multiboot2_header_end - multiboot2_header
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_ARCH +  MULTIBOOT_LENGTH)

.section .boot
.code32

.global C_SYMBOL(x86_start)
C_SYMBOL(x86_start):
  jmp  entry

/* for GRUB2 */
.align 8
multiboot2_header:
  .long MULTIBOOT_MAGIC
  .long MULTIBOOT_ARCH
  .long MULTIBOOT_LENGTH
  .long MULTIBOOT_CHECKSUM

  /* tag: information request */
info_req:
  .short 1
  .short 0
  .long  info_req_end - info_req
  .long  6 // memory map
  .long  9 // ELF symbol table
info_req_end:

 /* tag: terminator */
  .short 0
  .short 0
  .long  8
multiboot2_header_end:

/* the genuine entry point */
entry:
  cli
  lea  esp, [stack]

  // save a pointer to the boot information from GRUB2
  lea  eax, [grub_boot_info]
  mov  [eax], ebx

  // reload GDT
  lgdt [gdtr]
  jmp  24, new_gdt

new_gdt:
  mov  ax, 16
  mov  ds, ax
  mov  es, ax

/*
*
*  check if long mode (IA-32e) available
*
*/
check_cpuid:
  // Is CPUID instruction available?
  pushfd
  pop  eax
  bts  eax, 21
  push eax
  popfd
  pushfd
  test eax, 1 << 21
  jz   long_mode_unavailable

  // Are Extended Feature Bits available?
  mov  eax, 0x80000000
  cpuid
  cmp  eax, 0x80000001
  jb   long_mode_unavailable

  // Is long mode available?
  mov  eax, 0x80000001
  cpuid
  test edx, 1 << 29
  jz   long_mode_unavailable
  jmp  check_cpuid_end

long_mode_unavailable:
  lea  esi, [long_mode_unavailable_msg]
  call err32

check_cpuid_end:


/*
 *
 *  create a temporary paging table
 *
 *    virtual address base   ->   physical address base
 *    0x0000_0000_0000_0000  ->   0x0000_0000_0000_0000
 *
 */
  // PML4
  mov  edi, PGTABLE_BASE_ADDR
  mov  eax, PGTABLE_BASE_ADDR + 0x1000 + 0b11
  mov  [edi], eax

  // PDPT
  mov  edi, PGTABLE_BASE_ADDR + 0x1000 // the beginning of PDPT
  mov  eax, PGTABLE_BASE_ADDR + 0x2000 + 0b11 // the begininng of page directory
  mov  ecx, 512 // create 512 PDPT entries

create_pdpt_entry:
  mov  [edi], eax
  add  edi, 8
  add  eax, 8 * 512 // sizeof(PD_entry) * 512
  loop create_pdpt_entry

  // page directory
  mov  edi, PGTABLE_BASE_ADDR + 0x2000 // the beginning of page directory
  mov  edx, 0          // upper 32-bit
  mov  eax, 0b10000011 // lower 32-bit: readable, writable, 2MB page
  mov  ecx, 512 * 512 // create 64 pages (512GB)

create_pd_entry:
    mov  [edi],   eax
    mov  [edi+4], edx
    add  edi, 8
    add  eax, 0x200000
    loop create_pd_entry

/*
*
*  go into long mode!
*
*/
go_into_long_mode:
  // disable paging
  mov  eax, cr0
  and  eax, 0x7fffffff
  mov  cr0, eax

  // enable PAE and PGE
  mov  eax, cr4
  or   eax, 0b10100000
  mov  cr4, eax

  // load the temporary paging table
  mov  eax, PGTABLE_BASE_ADDR
  mov  cr3, eax

  // enable long mode
  mov  ecx, 0xc0000080 // read EFER
  rdmsr
  or   eax, 0x00000100 // EFER.LME = 1
  wrmsr

  // enable paging
  mov  eax, cr0
  or   eax, 0x80000000
  mov  cr0, eax

  // go into long mode
  jmp 8, long_mode

err32:
  mov  edi, 0xb8000
1:
  mov  al, [esi]
  test al, al
  jz   2f
  mov  [edi], al
  add  esi, 1
  add  edi, 2
  jmp  1b
2:
  cli
  hlt

long_mode_unavailable_msg: .asciz "The CPU does not support 64-bit mode!"

.code64
long_mode:
  mov  ax, 0
  mov  ds, ax
  mov  es, ax
  mov  fs, ax
  mov  gs, ax
  mov  ss, ax

/*
 *
 *  create a temporary paging table (used in initialization)
 *
 *    virtual address base   ->   physical address base
 *    0xffff_8000_0000_0000  ->   0x0000_0000_0000_0000
 *
 */
  // PML4
  mov  edi, PGTABLE_BASE_ADDR + (((KERNEL_BASE_ADDR >> 39) & 0x1ff) * 8)
  mov  eax, PGTABLE_BASE_ADDR + 0x1000 + 0b11
  mov  [edi], eax


/*
 *  go into x86_early_init()
 *
 */
go_x86_early_init:
  // FIXME: prepare stack for each processor
  lea    rsp, [stack]
  movabs rax, 0xffff800000000000
  add    rsp, rax

  // XXX
  mov  edi, [grub_boot_info]
  mov  rax, KERNEL_BASE_ADDR
  lea  rbx, [x86_init_entrypoint]
  add  rax, rbx
  call rax // _x86_early_init(&grub_boot_info)

halt:
  hlt
  jmp  halt


/* GDTR */
.align 8
gdtr:
  .word gdt_end - gdt - 1
  .quad gdt

/* GDT */
gdt:
  .quad 0x0000000000000000 // 0:  null descriptor
  .quad 0x00af9a000000ffff // 8:  64-bit code segment (kernel): 0x0000000000000000 - 0xffffffffffffffff
  .quad 0x00cf92000000ffff // 16: 64-bit data segment (kernel): 0x0000000000000000 - 0xffffffffffffffff
  .quad 0x00cf9a000000ffff // 24: 32-bit code segment (kernel): 0x00000000         - 0xffffffff
gdt_end:

/* GRUB */
grub_boot_info: .long 0

/* temporary stack (4KB) */
.space 4098
stack:

x86_init_entrypoint:

