#ifndef __KERNEL_RESOURCES_H__
#define __KERNEL_RESOURCES_H__

#include "process.h"

struct runqueue {
    struct runqueue *next;
    struct thread *thread;
};

struct resources {
    struct process *processes;
    struct runqueue *runqueue;
    mutex_t processes_lock;
    mutex_t runqueue_lock;
};

extern struct resources *resources;

#endif
