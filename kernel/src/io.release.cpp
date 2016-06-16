#include "kernel.h"
#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>

namespace kernel {
namespace io_server {

/** handles io.release */
void handle_release(channel_t __ch, resea::interfaces::io::space_t iospace, uintptr_t addr) {

    // TODO
}

} // namespace io_server
} // namespace kernel
