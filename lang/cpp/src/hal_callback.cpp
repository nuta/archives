#include "cpp.h"
#include <hal.h>
#include <resea.h>
#include <stdarg.h>


result_t call_hal_callback(enum hal_callback_type type, ...) {
    void *handler;
    va_list vargs;
    result_t r;

    va_start(vargs, type);
    handler = hal_get_callback(type);

    if (handler) {
        switch (type) {
	case HAL_CALLBACK_RUN_THREAD:
	{
            ident_t group    = va_arg(vargs, ident_t);
	    const char *name = va_arg(vargs, const char *);
	    uintptr_t entry  = va_arg(vargs, uintptr_t);
	    uintptr_t arg    = va_arg(vargs, uintptr_t);
            result_t (*_handler)(ident_t, const char *, uintptr_t, uintptr_t) = 
                (result_t (*)(ident_t, const char*, uintptr_t, uintptr_t)) handler;
            r = _handler(group, name, entry, arg);
	    break;
	}
	case HAL_CALLBACK_ALLOCATE_MEMORY:
	{
	    paddr_t   paddr    = va_arg(vargs, paddr_t);
	    size_t    size     = va_arg(vargs, size_t);
	    uint32_t  flags    = va_arg(vargs, uint32_t);
	    uintptr_t *addr    = va_arg(vargs, uintptr_t *);
	    uintptr_t *r_paddr = (uintptr_t *) va_arg(vargs, paddr_t *);
            result_t (*_handler)(paddr_t, size_t, uint32_t, uintptr_t *, paddr_t *) =
                (result_t (*)(paddr_t, size_t, uint32_t, uintptr_t *, paddr_t *)) handler;
            r = _handler(paddr, size, flags, addr, r_paddr);
	    break;
	}
        default:
            result_t (*_handler)() = (result_t (*)()) handler;
	    r = _handler();
	}
    } else {
        BUG("hal handler (type=%d) is not set");
	r = E_NOTFOUND;
    }

    va_end(vargs);
    return r;
}
