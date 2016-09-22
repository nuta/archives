#include "stdlibs.h"
#include <arch.h>
#include <logging.h>


void arch_panic(void) {

    WARN("PANIC!\n");
    exit(1);
}
