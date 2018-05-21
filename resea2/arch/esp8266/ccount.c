#include <arch.h>

uintmax_t arch_get_cycle_count(void) {
  uint32_t ccount;

    __asm__ __volatile__("rsr %0, ccount" : "=a"(ccount));
    return ccount;
}
