#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <hal.h>


/** handles memory.get_page_size */
void kernel_memory_get_page_size(channel_t __ch) {

    send_memory_get_page_size_reply(__ch, PAGE_SIZE);
}
