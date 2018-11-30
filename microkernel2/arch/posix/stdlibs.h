#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>


// FIXME: Because link(2) and link() in kernel/message.c conflict,
//        we cannot include unistd.h.
int usleep(unsigned int usec);
