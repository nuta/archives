#include "cpp.h"
#include <resea.h>


uint8_t asm_in8(uintptr_t addr) {

    BUG("port-mapped IO is not supported in ARM");
    return 0;
}


uint16_t asm_in16(uintptr_t addr) {

    BUG("port-mapped IO is not supported in ARM");
    return 0;
}


uint32_t asm_in32(uintptr_t addr) {

    BUG("port-mapped IO is not supported in ARM");
    return 0;
}


void asm_out8(uintptr_t addr, uint8_t data) {

    BUG("port-mapped IO is not supported in ARM");
}


void asm_out16(uintptr_t addr, uint16_t data) {

    BUG("port-mapped IO is not supported in ARM");
}


void asm_out32(uintptr_t addr, uint32_t data) {

    BUG("port-mapped IO is not supported in ARM");
}

