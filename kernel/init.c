#include "memory.h"
#include "printk.h"
#include "process.h"
#include "thread.h"
#include "server.h"
#include "string.h"
#include "init.h"
#include "test.h"
#include "kfs.h"

void kernel_init(void) {
    printk("\r\n");
    INFO("Starting Resea version " VERSION);

    memory_init();
    arch_early_init();
    process_init();
    thread_init();
    arch_init();

#ifdef KERNEL_TEST
    kernel_test();
#else
    kfs_init();
    kernel_server_init();
    launch_servers();
#endif

    if (thread_list_is_empty(&kernel_process->threads)) {
        PANIC("No threads to run.");
    }

    thread_switch();

    // From here, we're in the idle thread context. The thread
    // is resumed when there are no other threads to run.
    for (;;) {
        // Sleep until an interrupt occurs in order not to heat up the computer
        // and to save money on electricity.
        arch_idle();
    }
}
