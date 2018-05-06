#include "memory.h"
#include "process.h"
#include "thread.h"
#include "kfs.h"
#include "elf.h"
#include "server.h"
#include "string.h"


static void launch_servers(void) {
    struct kfs_dir dir;
    struct kfs_file file;
    kfs_opendir(&dir);
    while (kfs_readdir(&dir, &file) != NULL) {
        if (!strncmp("/servers/", file.name, 9)) {
            INFO("kernel: starting %s", file.name);
            elf_create_process(file.data, file.length, kfs_pager, file.pager_arg);
        }
    }
}


void kernel_init(void) {
    INFO("Starting Resea...");
    INFO("kernel: initializing memory system");
    memory_init();
    arch_early_init();

    INFO("kernel: initializing process system");
    process_init();

    INFO("kernel: initializing thread system");
    thread_init();

    INFO("kernel: initializing kfs system");
    kfs_init();
    arch_init();

    INFO("kernel: lauching kernel servers");
    kernel_server_init();

    INFO("kernel: lauching servers in kfs");
    launch_servers();

    if (thread_list_is_empty(&kernel_process->threads)) {
        PANIC("No threads to run.");
    }

    thread_switch();

    // From here, we're in the idle thread context. The thread
    // is resumed when there are no other threads to run.
    for (;;) {
        PANIC("idling...\n");
        // Sleep until an interrupt occurs in order not to heat up the computer
        // and to save money on electricity.
        arch_idle();
    }
}
