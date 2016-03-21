#include <hal.h>

result_t IORead8(uint8_t iospace, uintmax_t base,
               offset_t offset, uint8_t *data){

    *data = hal_io_read8(iospace, base, offset);
    return OK;
}


result_t IORead16(uint8_t iospace, uintmax_t base,
                offset_t offset, uint16_t *data){

    *data = hal_io_read16(iospace, base, offset);
    return OK;
}


result_t IORead32(uint8_t iospace, uintmax_t base,
                offset_t offset, uint32_t *data){

    *data = hal_io_read32(iospace, base, offset);
    return OK;
}


result_t IOWrite8(uint8_t iospace, uintmax_t base,
               offset_t offset, uint8_t data){

    hal_io_write8(iospace, base, offset, data);
    return OK;
}

result_t IOWrite16(uint8_t iospace, uintmax_t base,
                offset_t offset, uint16_t data){

    hal_io_write16(iospace, base, offset, data);
    return OK;
}


result_t IOWrite32(uint8_t iospace, uintmax_t base,
                offset_t offset, uint32_t data){

    hal_io_write32(iospace, base, offset, data);
    return OK;
}
