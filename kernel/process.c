#include "memory.h"
#include "process.h"

struct process *process_create(void) {
    struct process *process = kmalloc(sizeof(*process), KMALLOC_NORMAL);
    process->pid = allocate_tid();
//    list_init(&process->threads);
    return process;
}

void process_destroy(struct process *process) {

}

void process_init(void) {
}
