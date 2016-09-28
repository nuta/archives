#ifndef __KERNEL_RESOURCES_H__
#define __KERNEL_RESOURCES_H__

#include "process.h"

#define MAX_THREAD_NUM 32

struct resources {
    struct process *processes;
    struct thread *runqueue[MAX_THREAD_NUM];
    size_t runqueue_num;
    mutex_t processes_lock;
    mutex_t runqueue_lock;
};

extern struct resources *resources;

#endif
