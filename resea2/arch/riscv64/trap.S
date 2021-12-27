.macro SAVE_REGS_ON_STACK
.endm

.macro RESTORE_REGS_ON_STACK
.endm

.global riscv64_trap_entry
riscv64_trap_entry:
    SAVE_REGS_ON_STACK

    call riscv64_trap_handler

    RESTORE_REGS_ON_STACK
    sret
