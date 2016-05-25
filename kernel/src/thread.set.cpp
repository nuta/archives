#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include "kernel.h"


/** handles thread.set */
void kernel_thread_set(channel_t __ch, ident_t thread, uintptr_t entry, uintptr_t arg, uintptr_t stack, size_t stack_size) {

    send_thread_set_reply(__ch, kernel_set_thread(thread, entry, arg, stack, stack_size));
}
