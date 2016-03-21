#include <hal.h>
#include <resea.h>
#include "x86.h"

uint8_t x86_timer_vector = 0;
struct x86_cpuvar x86_cpuvars[CPU_MAX_NUM];
