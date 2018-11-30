#include "x86.h"
#include <hal.h>

#define EXCEPTION_PANIC(msg)           exp(msg, rip)
#define EXCEPTION_PANIC_W_ERRCODE(msg) exp2(msg, rip, errcode)


static void exp (const char *msg, uint64_t rip) {

    PANIC("%s: rip=0x%016x", msg, rip);
}


static void exp2 (const char *msg, uint64_t rip,
                  uint64_t errcode) {

    PANIC("%s: rip=0x%016x, errcode=%d", msg, rip, errcode);
}


void x86_exp_handler0 (uint64_t rip) {

    EXCEPTION_PANIC("#DE: Divide Error Exception");
}


void x86_exp_handler1 (uint64_t rip) {

    EXCEPTION_PANIC("#DB: Debug Exception");
}


void x86_exp_handler2 (uint64_t rip) {

    EXCEPTION_PANIC("NMI Interrupt");
}


void x86_exp_handler3 (uint64_t rip) {

    EXCEPTION_PANIC("#BP: Breakpoint Exception");
}


void x86_exp_handler4 (uint64_t rip) {

    EXCEPTION_PANIC("#OF: Overflow Exception");
}


void x86_exp_handler5 (uint64_t rip) {

    EXCEPTION_PANIC("#BR: BOUND Range Exceeded Exception");
}


void x86_exp_handler6 (uint64_t rip) {

    EXCEPTION_PANIC("#UD: Invalid Opcode Exception");
}


void x86_exp_handler7 (uint64_t rip) {

    EXCEPTION_PANIC("#NM: Device Not Available Exception");
}


void x86_exp_handler8 (uint64_t rip, uint64_t errcode) {

    EXCEPTION_PANIC_W_ERRCODE("#DF: Double Fault Exception");
}


void x86_exp_handler9 (uint64_t rip) {

    EXCEPTION_PANIC("Coprocessor Segment Overrun");
}


void x86_exp_handler10 (uint64_t rip, uint64_t errcode) {

    EXCEPTION_PANIC_W_ERRCODE("#TS: Invalid TSS Exception");
}


void x86_exp_handler11 (uint64_t rip, uint64_t errcode) {

    EXCEPTION_PANIC_W_ERRCODE("#NP: Segment Not Present");
}


void x86_exp_handler12 (uint64_t rip, uint64_t errcode) {

    EXCEPTION_PANIC_W_ERRCODE("#SS: Stack Fault");
}


void x86_exp_handler13 (uint64_t rip, uint64_t errcode) {

    EXCEPTION_PANIC_W_ERRCODE("#GP: General Protection Fault");
}


void x86_exp_handler14 (uint64_t rip, uint64_t errcode) {
    uint32_t reason;

    INFO("page fault: rip=%p, errcode=%#x", rip, errcode);

    if (errcode & 8) {
      PANIC("page fault with RSVD is set in the error code: "
                "this may be a bug in creating a page table");
    }

    reason  = 0;
    reason |= (errcode & (1 << 0))? PGFAULT_PRESENT : 0;
    reason |= (errcode & (1 << 1))? PGFAULT_WRITE   : 0;
    reason |= (errcode & (1 << 2))? PGFAULT_USER    : 0;
    reason |= (errcode & (1 << 4))? PGFAULT_EXEC    : 0;

    call_hal_callback(HAL_CALLBACK_PAGE_FAULT,
        x86_asm_read_cr2(), reason);
}


void x86_exp_handler15 (uint64_t rip) {

    EXCEPTION_PANIC("Unknown Exception 15 (reserved)");
}


void x86_exp_handler16 (uint64_t rip) {

    EXCEPTION_PANIC("#MF: x87 FPU Floating-Point Error");
}


void x86_exp_handler17 (uint64_t rip, uint64_t errcode) {

    EXCEPTION_PANIC_W_ERRCODE("#AC: Alignment Check Exception");
}


void x86_exp_handler18 (uint64_t rip) {

    EXCEPTION_PANIC("#MC: Machine Check Exception");
}


void x86_exp_handler19 (uint64_t rip) {

    EXCEPTION_PANIC("#XM: SIMD Floating-Point Exception");
}


void x86_exp_handler20 (uint64_t rip) {

    EXCEPTION_PANIC("#VE: Virtualization Exception");
}
