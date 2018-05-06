#ifndef __X64_ASM_H__
#define __X64_ASM_H__

#include <kernel/types.h>

static inline void asm_outb(u16_t port, u8_t value) {
    INLINE_ASM("outb %0, %1" :: "a"(value), "Nd"(port));
}

static inline u8_t asm_inb(u16_t port) {
    u8_t value;

    INLINE_ASM("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Don't divide into asm_sti and asm_hlt!
static inline void asm_stihlt(void) {
    INLINE_ASM("sti; hlt");
}

static inline void asm_cli(void) {
    INLINE_ASM("cli");
}

static inline void asm_hlt(void) {
    INLINE_ASM("hlt");
}

static inline void asm_lgdt(uptr_t gdtr) {
    INLINE_ASM("lgdt (%%rax)" :: "a"(gdtr));
}

static inline void asm_lidt(uptr_t idtr) {
    INLINE_ASM("lidt (%%rax)" :: "a"(idtr));
}

static inline void asm_ltr(u16_t tr) {
    INLINE_ASM("ltr %0" :: "a"(tr));
}

static inline void asm_set_cr3(u64_t value) {
    INLINE_ASM("mov %0, %%cr3" :: "r"(value) : "memory");
}

static inline u64_t asm_read_cr2(void) {
    u64_t value;
    INLINE_ASM("mov %%cr2, %0" : "=r"(value));
    return value;
}

static inline void asm_wrmsr(u32_t reg, u64_t value) {
    u32_t low = value & 0xffffffff;
    u32_t hi = value >> 32;
    INLINE_ASM("wrmsr" :: "c"(reg), "a"(low), "d"(hi));
}

static inline u64_t asm_rdmsr(u32_t reg) {
    u32_t low, high;
    INLINE_ASM("rdmsr" : "=a"(low), "=d"(high) : "c"(reg));
    return ((u64_t) high << 32) | low;
}

static inline void asm_invlpg(u64_t vaddr) {
    // Specify "memory" in clobber list to prevent memory
    // access reordering.
    INLINE_ASM("invlpg (%0)" :: "b"(vaddr) : "memory");
}

#endif
