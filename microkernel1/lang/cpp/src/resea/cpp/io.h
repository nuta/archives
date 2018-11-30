#ifndef __CPP_IO_H__
#define __CPP_IO_H__

#include <resea/io.h>

#define MEMORY_BARRIER() __asm__ __volatile__("": : :"memory")
#define PTR2ADDR(x) ((uintptr_t) (x))

uint8_t io_read8(resea::interfaces::io::space_t iospace, uintmax_t base, offset_t offset);
uint16_t io_read16(resea::interfaces::io::space_t iospace, uintmax_t base, offset_t offset);
uint32_t io_read32(resea::interfaces::io::space_t iospace, uintmax_t base, offset_t offset);
void io_write8(resea::interfaces::io::space_t iospace, uintmax_t base, offset_t offset, uint8_t data);
void io_write16(resea::interfaces::io::space_t iospace, uintmax_t base, offset_t offset, uint16_t data);
void io_write32(resea::interfaces::io::space_t iospace, uintmax_t base, offset_t offset, uint32_t data);

#endif
