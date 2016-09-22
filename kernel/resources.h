#ifndef __KERNEL_RESOURCES_H__
#define __KERNEL_RESOURCES_H__

#include <queue.h>
#include "process.h"

struct resources {
    struct process *processes;
    mutex_t processes_lock;
    struct queue run_queue;
};

extern struct resources *resources;

#endif
