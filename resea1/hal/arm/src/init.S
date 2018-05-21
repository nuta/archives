.section .boot
.global arm_init
arm_init:
    ldr    r0, =0xd3
    msr    cpsr, r0
    ldr    sp, =0x02000000

    bl arm_machine_startup
    bl arm_startup
    b .
