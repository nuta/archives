#include <arch.h>
#include "hypercall.h"


void arch_printchar(const char ch) {

    hypercalls->printchar(ch);
}
