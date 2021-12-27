.section .boot, "ax"
.global boot
boot:
    csrr a1, mhartid
    bne  a1, zero, not_first_cpu
    la   sp, stack_top
    j    qemu_machine_boot

not_first_cpu:
    wfi
    j not_first_cpu

// A stack space for the boot code and the idle thread afterwards.
// Allocate 256KiB since a debug build would consume too much stack.
stack_bottom:
.space 256 * 1024
stack_top:
