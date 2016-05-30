#include "elf.h"
#include <resea.h>
#include <resea/elf.h>
#include <resea/exec.h>
#include "handler.h"


void elf_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(exec, create):
        DEBUG("received exec.create");
        elf_exec_create(
            __ch
            , (uchar_t*) EXTRACT(m, exec, create, name)
            , (size_t) EXTRACT(m, exec, create, name_size)
            , (channel_t) EXTRACT(m, exec, create, fs)
            , (ident_t) EXTRACT(m, exec, create, file)
            , (ident_t) EXTRACT(m, exec, create, group)
        );
        return;
    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}
