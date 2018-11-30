#include "../../src/arm.h"
#include "raspi2.h"
#include <arm/machine.h>

extern "C" void arm_machine_startup() {

    arm::raspi2::init_timer();
}
