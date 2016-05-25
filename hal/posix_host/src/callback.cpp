#include "posix_host.h"
#include <hal.h>
#include <resea.h>
#include <stdarg.h>


static void * handlers[HAL_CALLBACK_MAX];

void * hal_get_callback(enum hal_callback_type type) {

    return handlers[type];
}

void hal_set_callback(enum hal_callback_type type, void * handler) {

    handlers[type] = handler;
}

