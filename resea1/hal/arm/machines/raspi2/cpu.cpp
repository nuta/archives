#include <hal.h>


// Refer http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0464f/index.html
cpuid_t hal_get_cpuid(void) {
    uint32_t mpidr;

    __asm__ __volatile__("mrc p15, 0, %0, c0, c0, 5" : "=r" (mpidr));
    return mpidr & 0b11;
}
