//
//  ARM Cortex-A7 Generic Timer driver
//
//    http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0464d/BABIGHII.html
//

#include "../../src/arm.h"
#include "raspi2.h"
#include <hal.h>
#include <arm/machine.h>


namespace arm {
namespace raspi2 {

#define CTRL_ENABLE      (1 << 0)
#define CTRL_INT_MASK    (1 << 1)

static void set_cntp_tval(uint32_t val) {

    ASM("mcr p15, 0, %[val], c14, c2, 0" :: [val] "r" (val));
}

static uint32_t get_cntp_ctrl() {
    uint32_t val;

    ASM("mrc p15, 0, %0, c14, c2, 1" : "=r" (val));
    return val;
}

static void set_cntp_ctrl(uint32_t val) {

    ASM("mcr p15, 0, %[val], c14, c2, 1" :: [val] "r" (val));
}

static uint32_t get_frequency() {
    uint32_t val;

    ASM("mrc p15, 0, %0, c14, c0, 0" : "=r" (val));
    return val;
}

static uint32_t i = 0;
void timer_handler() {

    set_cntp_tval(0x40000);

    if (!is_hal_callback_set(HAL_CALLBACK_RESUME_NEXT_THREAD))
        return;

    call_hal_callback(HAL_CALLBACK_TIMER_TICK);
}


void init_timer() {

    INFO("activating a generic timer (CNTP): frequency=%d",
         get_frequency());

    set_cntp_tval(0x4000);
    set_cntp_ctrl((get_cntp_ctrl() | CTRL_ENABLE) & (~CTRL_INT_MASK));

    *TIMER_CTRL(0) = 1;
    arm_accept_irq();
}

} // namesapce raspi2
} // namespace arm
