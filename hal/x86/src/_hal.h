#ifndef ___HAL_H__
#define ___HAL_H__

#include <resea.h>
#include "x86.h"


/*
 *  Memory
 */
#define PAGE_SIZE  4096

struct hal_vm_space{
  mutex_t    lock;
  uint64_t*  pml4;
};


/*
 *  Threading
 */
struct hal_thread{
  bool is_kernel;
  struct x86_thread_regs regs;
  void* fregs; // FXSAVE area, MUST be aligned to 16-byte boundary
};


/*
 *  CPU
 */
typedef uintmax_t cpuid_t;

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

void *kernel_allocate_memory(size_t size, uint32_t flags); // TODO: remove

#endif
