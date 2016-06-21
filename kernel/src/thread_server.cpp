#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include "kernel.h"


namespace kernel {
namespace thread_server {

/** handles thread.create */
void handle_create(channel_t __ch, ident_t group, uchar_t* name, size_t name_size) {
    result_t r;
    ident_t thread, r_group;

    r = kernel_create_thread(group, name, name_size,
                           &thread, &r_group);

    resea::interfaces::thread::send_create_reply(__ch, r, thread, r_group);
}


/** handles thread.delete */
void handle_delete(channel_t __ch, ident_t thread) {

}


/** handles thread.get_current_thread */
void handle_get_current_thread(channel_t __ch) {

}


/** handles thread.set */
void handle_set(channel_t __ch, ident_t thread, uintptr_t entry, uintptr_t arg, uintptr_t stack, size_t stack_size) {

    resea::interfaces::thread::send_set_reply(__ch, kernel_set_thread(thread, entry, arg, stack, stack_size));
}


/** handles thread.start */
void handle_start(channel_t __ch, ident_t thread) {

    resea::interfaces::thread::send_start_reply(__ch, kernel_set_thread_status(thread, THREAD_RUNNABLE));
}

} // namespace thread_server
} // namespace kernel
