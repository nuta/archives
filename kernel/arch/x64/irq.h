#ifndef __X64_IRQ_H__
#define __X64_IRQ_H__

#include <kernel/types.h>

// The interval must be long enough; short interval leads to
// recursive timer interrupt handling, kernel stack starvation,
// and eventually a nasty bug.
#define THREAD_SWITCH_INTERVAL 100

void x64_handle_irq(u8_t vector);

#endif
