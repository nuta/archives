#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"
#include "file.h"


/** handles fs.read */
void fat_fs_read(channel_t __ch,
       ident_t file,
       offset_t offset,
       size_t size) {

    result_t result;
    void* data;
    size_t r_size;

    data   = allocMemory(size, ALLOCMEM_NORMAL); // TODO: free on failure
    result = fat_read(&fat_the_disk, fat_get_file_by_id(file),
                      offset, data, size, &r_size);

    send_fs_read_reply(__ch, result, MOVE(data), size);
}
