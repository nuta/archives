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

/** handles fs.read */
void handle_read(channel_t __ch,
       ident_t file,
       offset_t offset,
       size_t size) {

    result_t result;
    void* data;
    size_t r_size;

    data   = allocate_memory(size, MEMORY_ALLOC_NORMAL); // TODO: free on failure
    result = fat::read(&fat::instance, fat_get_file_by_id(file),
                       offset, data, size, &r_size);

    sendas_fs_read_reply(__ch,
        result, PAYLOAD_INLINE,
        data,   PAYLOAD_MOVE_OOL,
        size,   PAYLOAD_INLINE);
}

} // namespace fs_server
} // namespace fat
