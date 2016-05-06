#include <resea.h>
#include <arm/machine.h>

enum {
    ARM_PINMODE_PULLDOWN_INPUT,
    ARM_PINMODE_ALT0
};


void delay_cycles(int cycles) {
    for (volatile int i=0; i < cycles; i++);
}

void pulldown(int pin) {
    volatile uint32_t *gppudclk;

    gppudclk = (pin <= 32)? ARM_GPIO_GPPUDCLK0 : ARM_GPIO_GPPUDCLK1;

    *ARM_GPIO_GPPUD = 0x01; // pulldown
    delay_cycles(150);
    *gppudclk = 1 << (pin % 32);
    delay_cycles(150);
    *gppudclk = 0;
}


void arm_set_pin_mode(int pin, int mode) {
    volatile uint32_t *gpfsel;

    if      (pin < 10) { gpfsel = ARM_GPIO_GPFSEL0; }
    else if (pin < 20) { gpfsel = ARM_GPIO_GPFSEL1; }
    else if (pin < 30) { gpfsel = ARM_GPIO_GPFSEL2; }
    else if (pin < 40) { gpfsel = ARM_GPIO_GPFSEL3; }
    else if (pin < 50) { gpfsel = ARM_GPIO_GPFSEL4; }
    else if (pin < 54) { gpfsel = ARM_GPIO_GPFSEL5; }
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
    static bool inited = false;

    if (!inited) {
	inited = true;

        *ARM_UART0_CR = 0;
	arm_set_pin_mode(14, ARM_PINMODE_PULLDOWN_INPUT);
	arm_set_pin_mode(15, ARM_PINMODE_PULLDOWN_INPUT);
	arm_set_pin_mode(14, ARM_PINMODE_ALT0);
	arm_set_pin_mode(15, ARM_PINMODE_ALT0);
        *ARM_UART0_IBRD = 1;
        *ARM_UART0_FBRD = 40;
        *ARM_UART0_LCRH = 0x70;
        *ARM_UART0_CR   = 0x0301;
    }

    while (*ARM_UART0_FR & (1 << 5));
    *ARM_UART0_DR = ch;
}

