#include <hal.h>


bool hal_interrupt_enabled(void) {

    return x86_asm_interrupt_enabled();
}


void hal_enable_interrupt(void) {

    x86_asm_sti();
}


void hal_disable_interrupt(void) {

    x86_asm_cli();
}


void hal_wait_interrupt (void) {

    x86_asm_stihlt();
}


void hal_enable_irq(uintmax_t irq) {
    uint8_t vector;

    vector = x86_alloc_intr_vector();
    x86_ioapic_enable_irq(vector, irq);
}

// Handles interrupts. Caller MUST disable interrupts
void x86_interrupt_handler (uint8_t vector) {

    x86_ack_interrupt();

    if (vector == x86_timer_vector)
        hal_call_callback(HAL_CALLBACK_TIMER_TICK);
    else
        hal_call_callback(HAL_CALLBACK_INTERRUPT, vector);
}

