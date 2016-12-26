#include <arch.h>
#include "hypercall_table.h"


void arch_printchar(const char ch) {

    hypercall_table->printchar(ch);
}
