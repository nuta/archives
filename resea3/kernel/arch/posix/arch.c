#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <kernel/types.h>

char __kfs;

void arch_copy_from_user(void *kernel, uptr_t user, size_t length) {
    abort();
}


void arch_idle(void) {

}

void arch_early_init(void) {
}

void arch_init(void) {
}

void arch_panic(void) {
    exit(0);
}


void arch_accept_irq(int irq) {
    abort();
}

void arch_create_vmspace(struct arch_vmspace *vms) {
}


void arch_destroy_vmspace(UNUSED struct arch_vmspace *vms) {
}


void arch_switch_vmspace(struct arch_vmspace *vms) {
}


void arch_link_page(struct arch_vmspace *vms, uptr_t vaddr, paddr_t paddr, size_t num,
                    int attrs) {
}

void arch_putchar(char ch) {
    putchar(ch);
}

void arch_allow_io(struct arch_thread *arch) {
}


void arch_destroy_thread(struct arch_thread *arch) {
}
