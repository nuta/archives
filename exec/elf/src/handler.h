#include "elf.h"
#include <resea.h>
#include <resea/exec.h>
#include <resea/elf.h>

namespace elf {
void server_handler(channel_t __ch, payload_t *m);

namespace exec_server {
void handle_create(
    channel_t __ch
    , uchar_t* name
    , size_t name_size
    , channel_t fs
    , ident_t file
    , ident_t group
);
} // namespace exec

} // namespace elf
