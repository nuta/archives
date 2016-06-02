#ifndef __ARM_ARM_H__
#define __ARM_ARM_H__

#define PACKAGE_NAME "arm"

#include <resea.h>
#include <hal.h>

#define CPU_MAX_NUM  32
#define CPUVAR ((struct arm_cpuvar *) &arm_cpuvars[hal_get_cpuid()])


struct arm_cpuvar {
  ident_t current_thread;
};


extern struct arm_cpuvar arm_cpuvars[CPU_MAX_NUM];

#endif
