#ifndef __ARM_MACHINE_H__
#define __ARM_MACHINE_H__

#define ARM_GPIO_GPFSEL0     ((volatile uint32_t *) 0x20200000)
#define ARM_GPIO_GPFSEL1     ((volatile uint32_t *) 0x20200004)
#define ARM_GPIO_GPFSEL2     ((volatile uint32_t *) 0x20200008)
#define ARM_GPIO_GPFSEL3     ((volatile uint32_t *) 0x2020000c)
#define ARM_GPIO_GPFSEL4     ((volatile uint32_t *) 0x20200010)
#define ARM_GPIO_GPFSEL5     ((volatile uint32_t *) 0x20200014)
#define ARM_GPIO_GPPUD       ((volatile uint32_t *) 0x20200094)
#define ARM_GPIO_GPPUDCLK0   ((volatile uint32_t *) 0x20200098)
#define ARM_GPIO_GPPUDCLK1   ((volatile uint32_t *) 0x2020009c)
#define ARM_UART0_DR         ((volatile uint32_t *) 0x20201000)
#define ARM_UART0_FR         ((volatile uint32_t *) 0x20201018)
#define ARM_UART0_IBRD       ((volatile uint32_t *) 0x20201024)
#define ARM_UART0_FBRD       ((volatile uint32_t *) 0x20201028)
#define ARM_UART0_LCRH       ((volatile uint32_t *) 0x2020102c)
#define ARM_UART0_CR         ((volatile uint32_t *) 0x20201030)

#endif
