#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "memory.h"
#include "thread.h"
#include "ipc.h"

#define HEAP_ADDR  0xc0000000
#define HEAP_ADDR_SIZE (0x40000000)
#define STACK_ADDR 0xf0000000

#define DEFAULT_CHANNELS_NUM 128

struct process {
    tid_t pid;
    uptr_t next_stack_start;
    struct thread *threads;
    struct vmspace vms;
    struct channel channels[DEFAULT_CHANNELS_NUM];
    size_t channels_max;
    kmutex_t lock;
};

extern struct process *kernel_process;

struct process *process_create(void);
void process_destroy(struct process *process);
void process_init(void);

#endif
