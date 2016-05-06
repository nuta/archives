#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include "kernel.h"


/** handles thread.start */
void kernel_thread_start(channel_t __ch, ident_t thread) {

    send_thread_start_reply(__ch, kernel_set_thread_status(thread, THREAD_RUNNABLE));
}
