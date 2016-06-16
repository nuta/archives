#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <hal.h>


namespace kernel {
namespace memory_server {

/** handles memory.get_page_size */
void handle_get_page_size(channel_t __ch) {

    resea::interfaces::memory::send_get_page_size_reply(__ch, PAGE_SIZE);
}

} // namespace memory_server
} // namespace kernel
