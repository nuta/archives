#ifndef __KERNEL_RESOURCES_H__
#define __KERNEL_RESOURCES_H__

#include "process.h"

struct resources {
    struct process *processes;
    struct thread *runqueue;
    mutex_t processes_lock;
    mutex_t runqueue_lock;
};

extern struct resources *resources;

#endif
