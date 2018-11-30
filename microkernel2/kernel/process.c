#include <logging.h>
#include "process.h"
#include "resources.h"
#include "thread.h"
#include "channel.h"
#include "kmalloc.h"
#include "list.h"


struct process *create_process(void) {
    struct process *process;

    // allocate memory for process and channels
    process = (struct process *) kmalloc(sizeof(struct process),
                                         KMALLOC_NORMAL);
    if (!process) {
        WARN("failed to kmalloc process");
        return NULL;
    }

    process->pid      = allocate_tid();
    process->threads  = NULL;
    process->channels = NULL;
    process->next     = NULL;
    mutex_init(&process->threads_lock);
    mutex_init(&process->channels_lock);

    // Update resources->processes
    mutex_lock(&resources->processes_lock);
    insert_into_list((struct list **) &resources->processes, process);
    mutex_unlock(&resources->processes_lock);

    return process;
}


void destroy_process(struct process *process) {
}
