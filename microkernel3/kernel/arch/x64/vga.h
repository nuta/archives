#ifndef __X64_VGA_H__
#define __X64_VGA_H__

#include <kernel/types.h>

#define COLOR 0x0f
#define X_MAX 80
#define Y_MAX 25

void x64_vga_putchar(char ch);
void x64_vga_init(void);

#endif
