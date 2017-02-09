#include <runtime.h>
#include <logging.h>


void init_cpp_runtime() {

    init_logging();
}


void destroy_cpp_runtime() {

}


extern "C" void __cxa_pure_virtual() {

    // TODO: ensure that logging is ready to use
    // TODO: add PANIC
    WARN("__cxa_pure_virtual");
}
