#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/pager.h>
#include <resea/fs.h>

namespace fat {
void server_handler(channel_t __ch, payload_t *m);

namespace fs_server {
void handle_open(
    channel_t __ch
    , uchar_t* path
    , size_t path_size
    , resea::interfaces::fs::filemode_t mode
);
} // namespace fs
namespace fs_server {
void handle_close(
    channel_t __ch
    , ident_t file
);
} // namespace fs
namespace fs_server {
void handle_read(
    channel_t __ch
    , ident_t file
    , offset_t offset
    , size_t size
);
} // namespace fs
namespace fs_server {
void handle_write(
    channel_t __ch
    , ident_t file
    , offset_t offset
    , void * data
    , size_t data_size
);
} // namespace fs
namespace pager_server {
void handle_fill(
    channel_t __ch
    , ident_t id
    , offset_t offset
    , size_t size
);
} // namespace pager

} // namespace fat
