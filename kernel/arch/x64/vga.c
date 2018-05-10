#include <kernel/string.h>
#include "asm.h"
#include "vga.h"

static void *vga_addr = NULL;
static int current_x, current_y;
#define VGA_PTR_Y(y) ((u8_t *) vga_addr + (y) * (X_MAX * 2))
#define VGA_PTR_YX(y, x) ((u8_t *) vga_addr + (y) * (X_MAX* 2) + ((x) * 2))


static void move_cursor_to(int y, int x) {
	int pos = y * X_MAX + x;

	asm_outb(0x3d4, 0x0f);
	asm_outb(0x3d5, pos & 0xff);
	asm_outb(0x3d4, 0x0e);
	asm_outb(0x3d5, (pos >> 8) & 0xff);
}

static void newline(void) {
    current_x = 0;
    current_y++;

    if (current_y == Y_MAX) {
        for (int i = 1; i < Y_MAX; i++) {
            u8_t *prev = VGA_PTR_Y(i - 1);
            u8_t *current = VGA_PTR_Y(i);
            memset(prev, 0x00, sizeof(u8_t) * X_MAX * 2);
            memcpy(prev, current, sizeof(u8_t) * X_MAX * 2);
        }

        current_y = Y_MAX - 1;
        u8_t *last = VGA_PTR_Y(current_y);
        memset(last, 0x00, sizeof(u8_t) * X_MAX * 2);
    }
}


void x64_vga_putchar(char ch) {
    if (current_x == X_MAX) {
        newline();
    }

    if (ch == '\n') {
        newline();
    } else {
        u8_t *p = (u8_t *) VGA_PTR_YX(current_y, current_x);
        p[0] = ch;
        p[1] = COLOR;
        current_x++;
    }

    move_cursor_to(current_y, current_x);
}

void x64_vga_init(void) {
    // FIXME: this requires a lock
    vga_addr = from_paddr(0xb8000);
    current_x = 0;
    current_y = 0;
}
