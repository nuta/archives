#include "fat.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "fat.h"
#include "file.h"

namespace fat {
namespace pager_server {

/** handles pager.fill */
void handle_fill(
         channel_t __ch,
         ident_t id,
         offset_t offset,
         size_t size) {

    result_t result;
    void* data;
    size_t r_size;

    data   = allocate_memory(size, MEMORY_ALLOC_NORMAL); // TODO: free on failure
    result = fat::read(&fat::instance, fat_get_file_by_id(id),
                       offset, data, size, &r_size);

    sendas_pager_fill_reply(__ch,
        result,  PAYLOAD_INLINE,
        data,    PAYLOAD_MOVE_OOL,
        r_size,  PAYLOAD_INLINE);
}

} // namespace pager_server
} // namespace fat
