#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include <resea/cpp/memory.h>
#include "fat.h"
#include "file.h"


namespace fat {
namespace fs_server {

/** handles fs.close */
void handle_close(channel_t __ch, ident_t file) {

    fat_free_file_by_id(file);
    resea::interfaces::fs::send_close_reply(__ch, OK);
}


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


/** handles fs.read */
void handle_read(channel_t __ch,
       ident_t file,
       offset_t offset,
       size_t size) {

    result_t result;
    void* data;
    size_t r_size;

    data   = allocate_memory(size, resea::interfaces::memory::ALLOC_NORMAL); // TODO: free on failure
    result = fat::read(&fat::instance, fat_get_file_by_id(file),
                       offset, data, size, &r_size);

    resea::interfaces::fs::sendas_read_reply(__ch,
        result, PAYLOAD_INLINE,
        data,   PAYLOAD_MOVE_OOL,
        size,   PAYLOAD_INLINE);
}


/** handles fs.write */
void handle_write(channel_t __ch,
       ident_t file,
       offset_t offset,
       void * data,
       size_t size) {

    /* TODO */
    resea::interfaces::fs::send_write_reply(__ch, E_NOTSUPPORTED);
}

} // namespace fs_server
} // namespace fat
