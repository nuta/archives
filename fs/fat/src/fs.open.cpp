#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"
#include "file.h"


/** handles fs.open */
void fat_fs_open(channel_t __ch, 
       uchar_t* path,
       size_t path_size,
       fs_filemode_t mode) {

    result_t result;
    ident_t file;

    file = fat_alloc_file_id();
    if (file == 0) {
        result = E_NOSPACE;
    }

    result = fat_open(&fat_the_disk, fat_get_file_by_id(file),
                      (const char *) path, path_size);

    send_fs_open_reply(__ch, result, file);
}
