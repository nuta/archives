#include "arm.h"
#include <resea.h>
#include <stdarg.h>

void start_apps(void);
extern "C" void kernel_startup(void);
void hal_printchar(char);

extern "C" void arm_startup(void) {

    kernel_startup();
    start_apps();
}
