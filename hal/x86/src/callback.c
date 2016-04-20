#include <hal.h>
#include <resea.h>
#include <stdarg.h>


static hal_handler_t handlers[HAL_CALLBACK_MAX];

hal_handler_t hal_get_callback(enum hal_callback_type type) {

    return handlers[type];
}

void hal_set_callback(enum hal_callback_type type, hal_handler_t handler) {

    handlers[type] = handler;
}

