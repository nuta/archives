#include <resea.h>
#include "cpp.h"

uint8_t io_read8(enum iospace_type iospace, uintmax_t base, offset_t offset) {

    switch (iospace) {
    case IOSPACE_PORT:
        return asm_in8(base + offset);
    case IOSPACE_MEM:
        return *((uint8_t *) base + offset);
    }

    WARN("unknown iospace, ignored");
    return 0;
}

uint16_t io_read16(enum iospace_type iospace, uintmax_t base, offset_t offset) {

    switch(iospace) {
    case IOSPACE_PORT:
        return asm_in16(base + offset);
    case IOSPACE_MEM:
        return *((uint16_t *) base + offset);
    }

    WARN("unknown iospace, ignored");
    return 0;
}

uint32_t io_read32(enum iospace_type iospace, uintmax_t base, offset_t offset) {

    switch(iospace) {
    case IOSPACE_PORT:
        return asm_in32(base + offset);
    case IOSPACE_MEM:
        return *((uint32_t *) base + offset);
    }

    WARN("unknown iospace, ignored");
    return 0;
}

void io_write8(enum iospace_type iospace, uintmax_t base, offset_t offset, uint8_t data) {

    switch(iospace) {
    case IOSPACE_PORT:
        asm_out8(base + offset, data);
        break;
    case IOSPACE_MEM:
        *((uint8_t *) base + offset) = data;
        break;
    default:
        WARN("unknown iospace, ignored");
    }
}

void io_write16(enum iospace_type iospace, uintmax_t base, offset_t offset, uint16_t data) {

    switch(iospace) {
    case IOSPACE_PORT:
        asm_out16(base + offset, data);
        break;
    case IOSPACE_MEM:
        *((uint16_t *) base + offset) = data;
        break;
    default:
        WARN("unknown iospace, ignored");
}
}

void io_write32(enum iospace_type iospace, uintmax_t base, offset_t offset, uint32_t data) {

    switch(iospace) {
    case IOSPACE_PORT:
        asm_out32(base + offset, data);
        break;
    case IOSPACE_MEM:
        *((uint32_t *) base + offset) = data;
        break;
    default:
        WARN("unknown iospace, ignored");
}
}

