#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "handler.h"


void fat_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgtype_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGTYPE(fs, open):
    {
        DEBUG("received fs.open");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            fat_fs_open(__ch, (uchar_t*) a0, (size_t) a1, (fs_filemode_t) a2);
            return;
    }
    case MSGTYPE(fs, close):
    {
        DEBUG("received fs.close");
            payload_t a0 = payloads[2];
            fat_fs_close(__ch, (ident_t) a0);
            return;
    }
    case MSGTYPE(fs, read):
    {
        DEBUG("received fs.read");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            fat_fs_read(__ch, (ident_t) a0, (offset_t) a1, (size_t) a2);
            return;
    }
    case MSGTYPE(fs, write):
    {
        DEBUG("received fs.write");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            fat_fs_write(__ch, (ident_t) a0, (offset_t) a1, (void *) a2, (size_t) a3);
            return;
    }
    case MSGTYPE(pager, fill):
    {
        DEBUG("received pager.fill");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            fat_pager_fill(__ch, (ident_t) a0, (offset_t) a1, (size_t) a2);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
