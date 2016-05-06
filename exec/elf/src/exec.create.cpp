#include <resea.h>
#include <resea/elf.h>
#include <resea/exec.h>
#include <resea/thread.h>
#include <resea/memory.h>
#include "elf.h"

static channel_t thread_ch = 0;
static channel_t memory_ch = 0;
static channel_t zeroed_pager_ch = 0;

/** handles exec.create */
void elf_exec_create(channel_t __ch, uchar_t* name, size_t name_size,
                     channel_t* fs, ident_t file, ident_t group) {
    result_t result;
    uintptr_t entry, stack;
    ident_t r_thread, r_group;

    /* connect to the servers if not connected */
    if (thread_ch == 0) {
        createchannel_t(&thread_ch);
        connect_channel(&thread_ch, INTERFACE(thread));
    }

    if (memory_ch == 0) {
        createchannel_t(&memory_ch);
        connect_channel(&memory_ch, INTERFACE(memory));
    }

    if (zeroed_pager_ch == 0) {
        createchannel_t(&zeroed_pager_ch);
        connect_channel(&zeroed_pager_ch, INTERFACE(zeroed_pager));
    }

    /* create a thread */
    INFO("creating a new thread");
    call_thread_create(&thread_ch,
         group, name, name_size,
         &result, &r_thread, &r_group);

    if (result != OK) {
        send_exec_create_reply(__ch, result, r_group, r_thread);
        return;
    }

    /* load the ELF file */
    INFO("loading an executable");
    if ((result = elf_load_executable(&memory_ch, r_group,
                                      fs, &zeroed_pager_ch, file,
                                      &entry, &stack)) != OK) {

        send_exec_create_reply(__ch, result, r_group, r_thread);
        return;
    }

    /* set registers states */
    call_thread_set(&thread_ch,
         r_thread, entry, (uintptr_t) nullptr, stack, STACK_SIZE,
         &result);

    send_exec_create_reply(__ch, result, r_group, r_thread);
}

