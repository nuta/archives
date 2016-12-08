#ifndef __ESP8266_ARCH_TYPES_H__
#define __ESP8266_ARCH_TYPES_H__

#include <types.h>

#define DEFAULT_THREAD_STACK_SIZE 2048
#define MUTEX_INITIALIZER  0
typedef tid_t mutex_t;
typedef unsigned int size_t;
#define SIZE_MAX 0xffffffff

struct arch_thread {
    uint32_t  pc;
    uint32_t  a1; // stack
    uint32_t  a0; // return address
    uint32_t  a2; // argument

    // callee-saved registers
    uint32_t  a12;
    uint32_t  a13;
    uint32_t  a14;
    uint32_t  a15;
};

#endif
