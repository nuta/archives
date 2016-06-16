#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"
#include "file.h"


namespace fat {
namespace fs_server {

/** handles fs.open */
void handle_open(channel_t __ch,
       uchar_t* path,
       size_t path_size,
       resea::interfaces::fs::filemode_t mode) {

    result_t result;
    ident_t file;

    file = fat_alloc_file_id();
    if (file == 0) {
        result = E_NOSPACE;
    }

    result = fat::open(&fat::instance, fat_get_file_by_id(file),
                       (const char *) path, path_size);

    resea::interfaces::fs::send_open_reply(__ch, result, file);
}

} // namespace fs_server
} // namespace fat
