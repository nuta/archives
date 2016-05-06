#include <resea.h>


uint8_t asm_in8(uintptr_t addr) {
     uint8_t r;

     __asm__ __volatile__("inb %1, %0" : "=a"(r) : "d"((uint16_t) addr));
     return r;
}


uint16_t asm_in16(uintptr_t addr) {
     uint16_t r;

     __asm__ __volatile__("inw %1, %0" : "=a"(r) : "d"((uint16_t) addr));
     return r;
}


uint32_t asm_in32(uintptr_t addr) {
     uint32_t r;

     __asm__ __volatile__("inl %1, %0" : "=a"(r) : "d"((uint16_t) addr));
     return r;
}


void asm_out8(uintptr_t addr, uint8_t data) {
    
     __asm__ __volatile__("outb %0, %1" :: "a"(data), "d"((uint16_t) addr));
}


void asm_out16(uintptr_t addr, uint16_t data) {
    
     __asm__ __volatile__("outw %0, %1" :: "a"(data), "d"((uint16_t) addr));
}


void asm_out32(uintptr_t addr, uint32_t data) {
    
     __asm__ __volatile__("outl %0, %1" :: "a"(data), "d"((uint16_t) addr));
}

