#include "fat.h"
#include <resea.h>
#include <resea/pager.h>
#include <resea/fs.h>
#include <resea/fat.h>

namespace fat {
namespace pager_server {

void handle_fill(
    channel_t __ch
    , ident_t id
    , offset_t offset
    , size_t size
);

} // namespace pager
} // namespace fat
