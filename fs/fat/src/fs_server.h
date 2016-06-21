#include "fat.h"
#include <resea.h>
#include <resea/pager.h>
#include <resea/fs.h>
#include <resea/fat.h>

namespace fat {
namespace fs_server {

void handle_open(
    channel_t __ch
    , uchar_t* path
    , size_t path_size
    , resea::interfaces::fs::filemode_t mode
);
void handle_close(
    channel_t __ch
    , ident_t file
);
void handle_read(
    channel_t __ch
    , ident_t file
    , offset_t offset
    , size_t size
);
void handle_write(
    channel_t __ch
    , ident_t file
    , offset_t offset
    , void * data
    , size_t data_size
);

} // namespace fs
} // namespace fat
