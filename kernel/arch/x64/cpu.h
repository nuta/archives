#ifndef __X64_CPU_H__
#define __X64_CPU_H__

#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "apic.h"

struct current_thread;
struct runqueue;

struct x64_cpuvar {
  struct seg_desc gdt[GDT_DESC_NUM];
  struct intr_desc idt[IDT_DESC_NUM];
  struct gdtr gdtr;
  struct idtr idtr;
  struct tss tss;
  struct thread *current;
  struct thread *idle_thread;
  struct runqueue *runqueue;
  kmutex_t runqueue_lock;
};

static inline int x64_get_cpu_index() {
    return (x64_read_apic(APIC_REG_ID) >> 24);
}

#define CPU_MAX_NUM 4
#define CPUVAR ((struct x64_cpuvar *) &x64_cpuvars[x64_get_cpu_index()])

extern struct x64_cpuvar x64_cpuvars[CPU_MAX_NUM];

#endif
