#include "elf.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/exec.h>
#include <resea/elf.h>
#include "handler.h"

namespace elf {

void server_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(exec, create):
        DEBUG("received exec.create");
        exec_server::handle_create(
            __ch
            , (uchar_t*) EXTRACT(m, exec, create, name)
            , (size_t) EXTRACT(m, exec, create, name_size)
            , (channel_t) EXTRACT(m, exec, create, fs)
            , (ident_t) EXTRACT(m, exec, create, file)
            , (ident_t) EXTRACT(m, exec, create, group)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
        release_memory((void * ) m[__PINDEX(m, exec, create, name)]);
        release_memory((void * ) m[__PINDEX(m, exec, create, name_size)]);
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}

} // namespace elf