#include <logging.h>
#include "process.h"
#include "resources.h"
#include "thread.h"
#include "channel.h"
#include "kmalloc.h"


struct process *create_process(void) {
    struct process *process;
    struct channel *channels;
    const size_t channels_max = DEFAULT_CHANNELS_MAX;

    // allocate memory for process and channels
    process = (struct process *) kmalloc(sizeof(struct process),
                                         KMALLOC_NORMAL);
    if (!process) {
        WARN("failed to kmalloc process");
        return NULL;
    }

    channels = (struct channel *) kmalloc(sizeof(struct channel) *
                                          channels_max, KMALLOC_NORMAL);
    if (!channels) {
        WARN("failed to kmalloc channels");
        kfree(process);
        return NULL;
    }

    // initialize channels
    for (cid_t cid = 1; cid <= channels_max; cid++) {
        channels[cid - 1].flags = CHANNEL_CLOSED;
    }

    process->pid           = allocate_tid();
    process->threads       = NULL;
    process->channels      = channels;
    process->channels_max  = channels_max;
    process->next          = NULL;
    mutex_init(&process->threads_lock);
    mutex_init(&process->channels_lock);

    // Update resources->processes
    mutex_lock(&resources->processes_lock);

    if (resources->processes) {
        struct process *proc = resources->processes;
        while (proc) {
            proc++;
        }

        proc->next = process;
    } else {
        resources->processes = process;
    }

    mutex_unlock(&resources->processes_lock);

    return process;
}


void destroy_process(struct process *process) {
}
