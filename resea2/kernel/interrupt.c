#include <resea/interrupt.h>
#include "event.h"
#include "interrupt.h"


void listen_interrupt(struct channel *ch, int vector) {

    listen_event(ch, INTERRUPT_INTERRUPT0 + vector, 0);
}
