#ifndef __KERNEL_INTERRUPT_H__
#define __KERNEL_INTERRUPT_H__

#include "channel.h"

void listen_interrupt(struct channel *ch, int vector);

#endif
