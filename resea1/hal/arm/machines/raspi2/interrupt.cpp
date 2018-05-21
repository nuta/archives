#include "../../src/arm.h"
#include "raspi2.h"
#include <arm/machine.h>


extern "C" void arm_irq_handler(struct hal_thread_regs *regs) {

    CPUVAR->irq_thread_state = regs;
    arm::raspi2::timer_handler();
}
