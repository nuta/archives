#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"
#include "file.h"


namespace fat {
namespace fs_server {

/** handles fs.close */
void handle_close(channel_t __ch, ident_t file) {

    fat_free_file_by_id(file);
    send_fs_close_reply(__ch, OK);
}

} // namespace fs_server
} // namespace fat
