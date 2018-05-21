#ifndef __ESP8266_THREAD_H__
#define __ESP8266_THREAD_H__

#include "arch_types.h"

NORETURN void esp8266_switch_thread(struct arch_thread *thread);

#endif
