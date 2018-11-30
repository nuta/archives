#include "kernel.h"
#include "thread.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>

using namespace kernel::thread;
using namespace resea::interfaces::thread;

namespace kernel {
namespace thread_server {


void handle_create(channel_t __ch, ident_t group, uchar_t* name, size_t name_size) {
    result_t r;
    ident_t thread, r_group;

    r = create_thread(group, name, name_size, &thread, &r_group);
    send_create_reply(__ch, r, thread, r_group);
}


void handle_delete(channel_t __ch, ident_t thread) {

}


void handle_get_current_thread(channel_t __ch) {

}


void handle_set(channel_t __ch, ident_t thread, uintptr_t entry, uintptr_t arg,
                uintptr_t stack, size_t stack_size) {

    send_set_reply(__ch, set_reg_state(thread, entry, arg, stack, stack_size));
}


void handle_start(channel_t __ch, ident_t thread) {

    send_start_reply(__ch, set_status(thread, THREAD_RUNNABLE));
}


} // namespace thread_server
} // namespace kernel
