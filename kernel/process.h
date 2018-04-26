#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "thread.h"

struct process {
    tid_t pid;
//    LIST(struct thread *) threads;
};

struct process *process_create(void);
void process_destroy(struct process *process);
void process_init(void);

#endif
