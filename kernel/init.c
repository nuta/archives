#include <arch.h>
#include <logging.h>
#include "init.h"
#include "panic.h"
#include "process.h"
#include "thread.h"
#include "resources.h"


extern uintptr_t apps[];
extern size_t app_stacks[];

void init_kernel(struct resources *_resources) {

    INFO("Welcome to Resea");

    resources = _resources;
    resources->processes = NULL;
    resources->runqueue  = NULL;
    mutex_init(&resources->processes_lock);
    mutex_init(&resources->runqueue_lock);

    INFO("creating the kernel process");
    struct process *kproc = create_process();

    INFO("creating in-kernel app");
    for (int i=0; apps[i]; i++) {
        INFO("in-kernel app: addr=%p", apps[i]);
        start_thread(create_thread(kproc, apps[i], 0, app_stacks[i]));
    }

    // Start the first thread
    struct thread *t = resources->runqueue->thread;
    if (!t) {
        PANIC("no apps to run");
    }

    INFO("starting the first thread");
    arch_switch_thread(t);
}
