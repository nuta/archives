#include <hal.h>

static void (*handlers[HAL_CALLBACK_MAX])();

void hal_call_callback(enum hal_callback_type type, ...) {
    void (*handler)() = handlers[type];

    if (handler)
        handler();
}


void hal_set_callback(enum hal_callback_type type, void (*handler)()) {

    handlers[type] = handler;
}

