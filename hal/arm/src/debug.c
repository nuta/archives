#include <resea.h>
#define GPIO_BASE   0x20200000
#define UART0_BASE  0x20201000

enum {
    ARM_PINMODE_PULLDOWN_INPUT,
    ARM_PINMODE_ALT0
};


void delay_cycles(int cycles) {
    for (volatile int i=0; i < cycles; i++);
}

void pulldown(int pin) {
    volatile uint32_t *gppudclk, *gppud;

    gppud    = (volatile uint32_t *) 0x20200094;
    gppudclk = (volatile uint32_t *) ((pin <= 32)? 0x20200098 : 0x2020009c);
    *gppud = 0x01; // pulldown
    delay_cycles(150);
    *gppudclk = 1 << (pin % 32);
    delay_cycles(150);
    *gppudclk = 0;
}


void arm_set_pin_mode(int pin, int mode) {
    volatile uint32_t *gpfsel;

    if      (pin < 10) { gpfsel = (volatile uint32_t *) 0x20200000; }
    else if (pin < 20) { gpfsel = (volatile uint32_t *) 0x20200004; }
    else if (pin < 30) { gpfsel = (volatile uint32_t *) 0x20200008; }
    else if (pin < 40) { gpfsel = (volatile uint32_t *) 0x2020000c; }
    else if (pin < 50) { gpfsel = (volatile uint32_t *) 0x20200010; }
    else if (pin < 54) { gpfsel = (volatile uint32_t *) 0x20200014; }
    else return; // invalid pin

    int shift = (pin % 10) * 3;

    switch (mode) {
    case ARM_PINMODE_PULLDOWN_INPUT:
	pulldown(pin);
	*gpfsel &= ~(7 << shift);
	break;
    case ARM_PINMODE_ALT0:
	*gpfsel |= 4 << shift;
	break;
    }
}

void hal_printchar(const char ch) {
    volatile uint32_t *cr, *ibrd, *fbrd, *lcrh, *fr, *dr;
    static bool inited = false;

    dr   = (volatile uint32_t *) 0x20201000;
    fr   = (volatile uint32_t *) 0x20201018;
    ibrd = (volatile uint32_t *) 0x20201024;
    fbrd = (volatile uint32_t *) 0x20201028;
    lcrh = (volatile uint32_t *) 0x2020102c;
    cr   = (volatile uint32_t *) 0x20201030;

    if (!inited) {
	inited = true;

        *cr = 0;
	arm_set_pin_mode(14, ARM_PINMODE_PULLDOWN_INPUT);
	arm_set_pin_mode(15, ARM_PINMODE_PULLDOWN_INPUT);
	arm_set_pin_mode(14, ARM_PINMODE_ALT0);
	arm_set_pin_mode(15, ARM_PINMODE_ALT0);
        *ibrd = 1;
        *fbrd = 40;
        *lcrh = 0x70;
        *cr   = 0x0301;
    }

    while(*fr & (1 << 5));
    *dr = ch;
}

