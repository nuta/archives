#include <runtime.h>
#include <logging.h>

#ifndef ARCH_POSIX

extern int __ctors;
extern int __ctors_end;
extern int __dtors;
extern int __dtors_end;


static void call_functions(uintptr_t start, uintptr_t end) {
    uintptr_t fn = start;

    while(fn < end) {
        INFO("ctor >>> %p", fn);
        fn += sizeof(void *);
    }
}

#endif // !ARCH_POSIX

void call_ctors() {

#ifndef ARCH_POSIX
    call_functions((uintptr_t) &__ctors, (uintptr_t)  &__ctors_end);
#endif // !ARCH_POSIX
}


void call_dtors() {

#ifndef ARCH_POSIX
    call_functions((uintptr_t) &__dtors, (uintptr_t)  &__dtors_end);
#endif // !ARCH_POSIX
}


void init_cpp_runtime() {

    init_logging();
    call_ctors();
}


void destroy_cpp_runtime() {

    call_dtors();
}
