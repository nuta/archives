#include <kernel/types.h>
#include "vga.h"
#include "serial.h"

void arch_putchar(char ch) {
    x64_serial_putchar(ch);
    x64_vga_putchar(ch);
}
