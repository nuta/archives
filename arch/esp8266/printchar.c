#include <arch.h>
#include "finfo.h"


void arch_printchar(const char ch) {

    finfo->printchar(ch);
}
