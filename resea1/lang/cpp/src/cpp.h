#ifndef __CPP_CPP_H__
#define __CPP_CPP_H__

#define PACKAGE_NAME "cpp"

#include <resea.h>
#include <resea/cpp/io.h>

uint8_t asm_in8(uintptr_t addr);
uint16_t asm_in16(uintptr_t addr);
uint32_t asm_in32(uintptr_t addr);
void asm_out8(uintptr_t addr, uint8_t data);
void asm_out16(uintptr_t addr, uint16_t data);
void asm_out32(uintptr_t addr, uint32_t data);

#endif
