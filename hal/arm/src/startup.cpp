#include "arm.h"
#include <resea.h>
#include <stdarg.h>

void start_apps(void);
void kernel_startup(void);


void arm_startup(void) {

    for(volatile int x=0; x < 0x100000; x++);

    *((volatile unsigned *) 0x20200004) = 1 << 18;

    for (int i=0; i < 4; i++) {
         hal_printchar('*');
         *((volatile unsigned *) 0x20200028) = 1 << 16;
         for(volatile int x=0; x < 0xa0000; x++);
         *((volatile unsigned *) 0x2020001c) = 1 << 16;
         for(volatile int x=0; x < 0xa0000; x++);
    }

    *((volatile unsigned *) 0x20200028) = 1 << 16;

    INFO("HELLO WORLD!");
    for(;;);
    kernel_startup();
    start_apps();
}
