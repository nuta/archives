#ifndef __ARM_MACHINE_H__
#define __ARM_MACHINE_H__

#define ARM_GPIO_GPFSEL0     ((volatile uint32_t *) 0x101e4000)
#define ARM_GPIO_GPFSEL1     ((volatile uint32_t *) 0x101e4004)
#define ARM_GPIO_GPFSEL2     ((volatile uint32_t *) 0x101e4008)
#define ARM_GPIO_GPFSEL3     ((volatile uint32_t *) 0x101e400c)
#define ARM_GPIO_GPFSEL4     ((volatile uint32_t *) 0x101e4010)
#define ARM_GPIO_GPFSEL5     ((volatile uint32_t *) 0x101e4014)
#define ARM_GPIO_GPPUD       ((volatile uint32_t *) 0x101e4094)
#define ARM_GPIO_GPPUDCLK0   ((volatile uint32_t *) 0x101e4098)
#define ARM_GPIO_GPPUDCLK1   ((volatile uint32_t *) 0x101e409c)
#define ARM_UART0_DR         ((volatile uint32_t *) 0x101f1000)
#define ARM_UART0_FR         ((volatile uint32_t *) 0x101f1018)
#define ARM_UART0_IBRD       ((volatile uint32_t *) 0x101f1024)
#define ARM_UART0_FBRD       ((volatile uint32_t *) 0x101f1028)
#define ARM_UART0_LCRH       ((volatile uint32_t *) 0x101f102c)
#define ARM_UART0_CR         ((volatile uint32_t *) 0x101f1030)

#endif
