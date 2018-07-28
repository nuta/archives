#include "list.h"
#include "memory.h"
#include "printk.h"
#include "process.h"


struct process *process_create(void) {
    struct process *process = kmalloc(sizeof(*process), KMALLOC_NORMAL);

    process->pid = allocate_tid();
    process->next_stack_start = STACK_ADDR;
    process->channels_max = DEFAULT_CHANNELS_NUM;

    for (size_t i = 0; i < DEFAULT_CHANNELS_NUM; i++) {
        process->channels[i].state = CHANNEL_UNUSED;
   }


    kmutex_init(&process->lock, KMUTEX_UNLOCKED);
    thread_list_init(&process->threads);
    memory_create_vmspace(&process->vms);

    return process;
}


void process_destroy(struct process *process) {
    kmutex_init(&process->lock, KMUTEX_LOCKED);

    // Close all channels.
    for (size_t i = 0; i < process->channels_max; i++) {
        if (process->channels[i].state == CHANNEL_OPENED) {
            channel_close(process->channels);
        }
    }

    // Destroy all threads.
    struct thread *thread = process->threads;
    while (thread != NULL) {
        struct thread *next = thread->next;
        thread_destroy(thread);
        thread = next;
    }

    // Release resources.
    arch_destroy_vmspace(&process->vms.arch);
    free_tid(process->pid);
}


struct process *kernel_process = NULL;
void process_init(void) {
    kernel_process = process_create();
}
