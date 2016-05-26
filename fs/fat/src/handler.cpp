#include "fat.h"
#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "handler.h"


void fat_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    while(!fat_initialized);

    switch (EXTRACT_MSGID(m)) {
    case MSGID(fs, open):
        DEBUG("received fs.open");
        fat_fs_open(
            __ch
            , (uchar_t*) EXTRACT(m, fs, open, path)
            , (size_t) EXTRACT(m, fs, open, path_size)
            , (fs_filemode_t) EXTRACT(m, fs, open, mode)
        );
        return;
    case MSGID(fs, close):
        DEBUG("received fs.close");
        fat_fs_close(
            __ch
            , (ident_t) EXTRACT(m, fs, close, file)
        );
        return;
    case MSGID(fs, read):
        DEBUG("received fs.read");
        fat_fs_read(
            __ch
            , (ident_t) EXTRACT(m, fs, read, file)
            , (offset_t) EXTRACT(m, fs, read, offset)
            , (size_t) EXTRACT(m, fs, read, size)
        );
        return;
    case MSGID(fs, write):
        DEBUG("received fs.write");
        fat_fs_write(
            __ch
            , (ident_t) EXTRACT(m, fs, write, file)
            , (offset_t) EXTRACT(m, fs, write, offset)
            , (void *) EXTRACT(m, fs, write, data)
            , (size_t) EXTRACT(m, fs, write, data_size)
        );
        return;
    case MSGID(pager, fill):
        DEBUG("received pager.fill");
        fat_pager_fill(
            __ch
            , (ident_t) EXTRACT(m, pager, fill, id)
            , (offset_t) EXTRACT(m, pager, fill, offset)
            , (size_t) EXTRACT(m, pager, fill, size)
        );
        return;
    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}
