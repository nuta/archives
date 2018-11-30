#include "./emscripten.h"
#include "../port.h"

uintptr_t arch_get_stack_bottom(void) {
    return 0;
}

void arch_load_regs(uintptr_t *regs) {
}
