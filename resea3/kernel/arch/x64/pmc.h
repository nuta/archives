#ifndef __X64_PMC_H__
#define __X64_PMC_H__

// Event Select & UMask
#define PMC_EVENT_UNHALTED_CORE_CYCLES             0x003c
#define PMC_EVENT_MEM_LOAD_UOPS_RETIRED_LLC_MISS   0x20d1

// Count when the processor is in user.
#define PMC_USER (1 << 16)

// Count when the processor is in kernel.
#define PMC_KERNEL (1 << 17)

#define PMC_ENABLE (1 << 22)

void x64_init_pmc(void);

#endif
