#ifndef __PORT_H__

#if defined(ENA_PORT_x64)
#   include "port/x64.h"
#elif defined(ENA_PORT_emscripten)
#   include "port/emscripten.h"
#else
#   error "specify port"
#endif

uintptr_t arch_get_stack_bottom(void);
void arch_load_regs(uintptr_t *regs);

#endif
