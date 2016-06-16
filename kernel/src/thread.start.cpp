#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include "kernel.h"


namespace kernel {
namespace thread_server {

/** handles thread.start */
void handle_start(channel_t __ch, ident_t thread) {

    resea::interfaces::thread::send_start_reply(__ch, kernel_set_thread_status(thread, THREAD_RUNNABLE));
}

} // namespace thread_server
} // namespace kernel
