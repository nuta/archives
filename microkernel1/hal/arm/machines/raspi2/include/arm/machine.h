#ifndef __ARM_MACHINE_H__
#define __ARM_MACHINE_H__

//
//  Raspberry Pi 2 uses BCM2836
//
//    https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/
//
//  Note that the base address differs from BCM2835.
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

#define PAGE_SIZE 4096

struct hal_vm_space {
};

struct hal_thread_regs {
    uint32_t  r0;
    uint32_t  r1;
    uint32_t  r2;
    uint32_t  r3;
    uint32_t  r4;
    uint32_t  r5;
    uint32_t  r6;
    uint32_t  r7;
    uint32_t  r8;
    uint32_t  r9;
    uint32_t  r10;
    uint32_t  r11;
    uint32_t  r12;
    uint32_t  lr;
    uint32_t  pc;
    uint32_t  sp;
    uint32_t  spsr;
} PACKED;

struct hal_thread {
    bool is_kernel;
    struct hal_thread_regs regs;
};

extern "C" {
void arm_asm_resume_thread(uint32_t sp, uint32_t spsr);
}

#endif
