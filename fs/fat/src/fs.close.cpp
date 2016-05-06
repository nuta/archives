#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"
#include "file.h"


/** handles fs.close */
void fat_fs_close(channel_t __ch, ident_t file) {

    fat_free_file_by_id(file);
    send_fs_close_reply(__ch, OK);
}
