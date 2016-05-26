#include "_x86.h"
#include <hal.h>


static unsigned int x = 0, y = 0;


static void serialport_send_char (char ch) {

    while((x86_asm_in8(IOPORT_SERIAL_STATUS) & 0x20) == 0);
    x86_asm_out8(IOPORT_SERIAL_DATA, (uint8_t) ch);
}


static void move_cursor (void) {
    unsigned int cursor = (y*SCREEN_X_MAX)+x;

    x86_asm_out8(0x03d4, 15);
    x86_asm_out8(0x03d5, cursor & 0xff);
    x86_asm_out8(0x03d4, 14);
    x86_asm_out8(0x03d5, (cursor >> 8) & 0xff);

}


static void vram_draw_char (char ch, char color) {

    if (ch == '\n') {
        y++;
        x = 0;

    } else {
        VRAM_CHAR_AT(y,  x) = ch;
        VRAM_COLOR_AT(y, x) = color;
        x++;

        if (x >= SCREEN_X_MAX) {
            y++;
            x = 0;
        }
    }

    if (y >= SCREEN_Y_MAX) {
        for (y=0; y < SCREEN_Y_MAX - 1; y++) {
            for (x=0; x < SCREEN_X_MAX; x++) {
                VRAM_CHAR_AT(y, x)  = VRAM_CHAR_AT(y + 1,  x);
                VRAM_COLOR_AT(y, x) = VRAM_COLOR_AT(y + 1, x);
            }
        }

        for (x=0; x < SCREEN_X_MAX; x++) {
            VRAM_CHAR_AT(SCREEN_Y_MAX - 1, x)  = ' ';
            VRAM_COLOR_AT(SCREEN_Y_MAX - 1, x) = 0x0f;
        }

        y = SCREEN_Y_MAX - 1;
        x = 0;
    }

    move_cursor();
}


void hal_printchar (char ch) {

    serialport_send_char(ch);
    vram_draw_char(ch, 0x0f /* white characters on a black background */);
}


void x86_init_print (void) {

    /* enable serial port */
    x86_asm_out8(IOPORT_SERIALPORT + 1, 0x00);
    x86_asm_out8(IOPORT_SERIALPORT + 3, 0x80);
    x86_asm_out8(IOPORT_SERIALPORT + 0, 0x03);
    x86_asm_out8(IOPORT_SERIALPORT + 1, 0x00);
    x86_asm_out8(IOPORT_SERIALPORT + 3, 0x03);
    x86_asm_out8(IOPORT_SERIALPORT + 2, 0xc7);
    x86_asm_out8(IOPORT_SERIALPORT + 4, 0x0b);
}

