#include "fat.h"
#include <resea.h>

void fat_fs_open(
    channel_t __ch
    , uchar_t* path
    , size_t path_size
    , fs_filemode_t mode
);
void fat_fs_close(
    channel_t __ch
    , ident_t file
);
void fat_fs_read(
    channel_t __ch
    , ident_t file
    , offset_t offset
    , size_t size
);
void fat_fs_write(
    channel_t __ch
    , ident_t file
    , offset_t offset
    , void * data
    , size_t data_size
);
void fat_pager_fill(
    channel_t __ch
    , ident_t id
    , offset_t offset
    , size_t size
);
