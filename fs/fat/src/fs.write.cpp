#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"

/** handles fs.write */
void fat_fs_write(channel_t __ch,
       ident_t file,
       offset_t offset,
       void * data,
       size_t size) {

    /* TODO */
    send_fs_write_reply(__ch, E_NOTSUPPORTED);
}
