#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include "kernel.h"


namespace kernel {
namespace thread_server {

/** handles thread.set */
void handle_set(channel_t __ch, ident_t thread, uintptr_t entry, uintptr_t arg, uintptr_t stack, size_t stack_size) {

    resea::interfaces::thread::send_set_reply(__ch, kernel_set_thread(thread, entry, arg, stack, stack_size));
}

} // namespace thread_server
} // namespace kernel
