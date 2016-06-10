.section .boot
.global vectors
vectors:
    ldr pc, reset_handler
    ldr pc, undefined_handler
    ldr pc, swi_handler
    ldr pc, prefetch_handler
    ldr pc, data_handler
    ldr pc, hyp_handler
    ldr pc, irq_handler
    ldr pc, fiq_handler

reset_handler:      .word reset
undefined_handler:  .word halt
swi_handler:        .word halt
prefetch_handler:   .word halt
data_handler:       .word halt
hyp_handler:        .word halt
irq_handler:        .word irq
fiq_handler:        .word halt

vectors_end:


reset:
    ldr  r0, =0xd3
    msr  cpsr, r0
    ldr  sp, =boot_stack

    // copy the vector table at 0x00000000
    mov  r0, #0x00
    ldr  r1, =vectors
    ldr  r2, =vectors_end

loop:
    ldr  r3, [r1, #0x00]
    str  r3, [r0, #0x00]
    add  r1, r1, #0x04
    add  r0, r0, #0x04
    cmp  r1, r2
    bne  loop

    // switch to IRQ mode
    mrs  r0, cpsr
    bic  r1, r0, #0x1f
    orr  r1, r1, #0x12
    msr  cpsr, r1

    // set the stack pointer in IRQ mode
    ldr  sp, =irq_stack

    // return to supervisor mode
    msr  cpsr, r0

    bl  arm_machine_startup
    bl  arm_startup
    b   halt
    b .

halt:
    wfi
    b halt

irq:
    push {r0, r1, r2, r3} // for storing lr, pc, sp and spsr
    push {r0-r12}

    // set the stack address for storing lr
    add  r0, sp, #0x34

    // save thread's spsr and pc
    mrs  r1, spsr
    str  r1, [r0, #0x0c]
    sub  r2, lr, #4
    str  r2, [r0, #0x04]

    // switch to Supervisor mode temporarily to
    // obtain lr, and sp that stored in banked registers
    // TODO: support User mode
    mrs  r1, cpsr
    bic  r2, r1, #0x1f
    orr  r2, r2, #0x13
    msr  cpsr, r1

    // save thread's lr and sp
    str  sp, [r0, #0x08]
    str  lr, [r0, #0x00]

    // go back to IRQ mode
    msr  cpsr, r1

    mov  r0, sp // the pointer to the begginig of saved state
    push {lr}
    bl   arm_irq_handler
    pop  {lr}

    // leave the IRQ handler
    pop  {r0-r12}
    sub  pc, lr, #4

