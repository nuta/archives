#include <logging.h>
#include <runtime.h>
#include "halt.h"


void halt_kernel(void) {

    INFO("halting Resea...");

    destroy_cpp_runtime();

    INFO("halted");
}
