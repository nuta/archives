#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"

namespace fat {
namespace fs_server {

/** handles fs.write */
void handle_write(channel_t __ch,
       ident_t file,
       offset_t offset,
       void * data,
       size_t size) {

    /* TODO */
    send_fs_write_reply(__ch, E_NOTSUPPORTED);
}

} // namespace fs_server
} // namespace fat
