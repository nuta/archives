#ifndef __ARM_MACHINE_H__
#define __ARM_MACHINE_H__

//
//  Raspberry Pi 2 uses BCM2836
//
//    https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/
//

#define ARM_GPIO_GPFSEL0     ((volatile uint32_t *) 0x3f200000)
#define ARM_GPIO_GPFSEL1     ((volatile uint32_t *) 0x3f200004)
#define ARM_GPIO_GPFSEL2     ((volatile uint32_t *) 0x3f200008)
#define ARM_GPIO_GPFSEL3     ((volatile uint32_t *) 0x3f20000c)
#define ARM_GPIO_GPFSEL4     ((volatile uint32_t *) 0x3f200010)
#define ARM_GPIO_GPFSEL5     ((volatile uint32_t *) 0x3f200014)
#define ARM_GPIO_GPPUD       ((volatile uint32_t *) 0x3f200094)
#define ARM_GPIO_GPPUDCLK0   ((volatile uint32_t *) 0x3f200098)
#define ARM_GPIO_GPPUDCLK1   ((volatile uint32_t *) 0x3f20009c)
#define ARM_UART0_DR         ((volatile uint32_t *) 0x3f201000)
#define ARM_UART0_FR         ((volatile uint32_t *) 0x3f201018)
#define ARM_UART0_IBRD       ((volatile uint32_t *) 0x3f201024)
#define ARM_UART0_FBRD       ((volatile uint32_t *) 0x3f201028)
#define ARM_UART0_LCRH       ((volatile uint32_t *) 0x3f20102c)
#define ARM_UART0_CR         ((volatile uint32_t *) 0x3f201030)

#endif
