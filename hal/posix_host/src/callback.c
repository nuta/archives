#include <hal.h>
#include <resea.h>
#include <stdarg.h>


static void (*handlers[HAL_CALLBACK_MAX])();

void hal_call_callback(enum hal_callback_type type, ...) {
    void (*handler)();
    va_list vargs;

    va_start(vargs, type);
    handler = handlers[type];

    if (handler) {
        switch (type) {
	case HAL_CALLBACK_RUN_THREAD:
	{
            ident_t group    = va_arg(vargs, ident_t);
	    const char *name = va_arg(vargs, const char *);
	    uintptr_t entry  = va_arg(vargs, uintptr_t);
	    uintptr_t arg    = va_arg(vargs, uintptr_t);
            handler(group, name, entry, arg);
	    break;
	}
        default:
	    handler();
	}
    } else {
        BUG("hal handler (type=%d) is not set");
    }

    va_end(vargs);
}


void hal_set_callback(enum hal_callback_type type, void (*handler)()) {

    handlers[type] = handler;
}

