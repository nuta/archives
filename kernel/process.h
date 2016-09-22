#ifndef __KERNEL_PROCESS_H__
#define __KERNEL_PROCESS_H__

#include <types.h>
#include <mutex.h>
#include "thread.h"

#define DEFAULT_CHANNELS_MAX 16

struct process {
    pid_t pid;
    mutex_t channels_lock;
    mutex_t threads_lock;
    struct channel *channels;
    size_t channels_max;
    struct thread  *threads;
    struct process *next;
};

struct process *create_process(void);
void destroy_process(struct process *process);

#endif
