#include <arch.h>
#include <queue.h>
#include <logging.h>
#include "init.h"
#include "process.h"
#include "thread.h"
#include "resources.h"


extern uintptr_t apps[];

void init_kernel(struct resources *_resources) {

    INFO("Welcome to Resea");

    resources = _resources;
    resources->processes = NULL;
    mutex_init(&resources->processes_lock);
    mutex_init(&resources->runqueue_lock);
    resources->runqueue_num = MAX_THREAD_NUM;
    for (int i = 0; i < resources->runqueue_num; i++) {
        resources->runqueue[i] = NULL;
    }

    INFO("creating the kernel process");
    struct process *kproc = create_process();

    INFO("creating in-kernel threads");
    for (int i=0; apps[i]; i++) {
        start_thread(create_thread(kproc, apps[i], 0));
    }

    // Start the first thread
    struct thread *t = resources->runqueue[0];

    INFO("starting the first thread");
    arch_switch_thread(t->tid, &t->arch);
}
