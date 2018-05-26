#include <kernel/types.h>
#include <kernel/string.h>
#include <kernel/init.h>
#include <kernel/printk.h>
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "cpuvar.h"
#include "pic.h"
#include "apic.h"
#include "paging.h"
#include "serial.h"
#include "vga.h"
#include "smp.h"
#include "syscall.h"


/* The bootstarap processor (the first processor) initialization. */
void x64_init_bsp(void) {
    // Note that the kernel memory allocator is not initialized yet.
    x64_vga_init();
    x64_init_serial();
    x64_init_pic();
    kernel_init();
}


/* The bootstarap processor run this once. */
void arch_early_init(void) {
    // Now we are able to use kernel memory allocator.

    INFO("x64: initializing paging");
    x64_init_paging();

    INFO("x64: initializing Local APIC");
    x64_init_apic();

    INFO("x64: initializing cpuvar");
    x64_init_cpuvar();

    INFO("x64: initializing GDT");
    x64_init_gdt();
    INFO("x64: initializing TSS");
    x64_init_tss();
    INFO("x64: initializing IDT");
    x64_init_idt();

    INFO("x64: initializing SMP");
    x64_init_smp();
    INFO("x64: initializing Local APIC Timer");
    x64_init_apic_timer();
    INFO("x64: initializing SYSCALL/SYSRET");
    x64_init_syscall();
}


/* The bootstarap processor run this once. */
void arch_init(void) {
    // All kernel components are initialized.
}
