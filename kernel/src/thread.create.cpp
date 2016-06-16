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

    send_thread_create_reply(__ch, r, thread, r_group);
}

} // namespace thread_server
} // namespace kernel
