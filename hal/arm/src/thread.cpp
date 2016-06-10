#include <hal.h>
#include "arm.h"


void hal_set_current_thread_id(ident_t id) {

    CPUVAR->current_thread = id;
}


ident_t hal_get_current_thread_id(void) {

    return CPUVAR->current_thread;
}
