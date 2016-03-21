#include <resea.h>
#include <resea/fat.h>
#include <resea/fs.h>
#include <resea/pager.h>
#include "handler.h"


void fat_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (service)");
        return;
    }

    switch (payloads[1]) {
    case SERVICE(fs, open):
    {
        DEBUG("received fs.open");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            fat_fs_open(__ch, (uchar_t*) a0, (fs_FileMode) a1);
            return;
    }
    case SERVICE(fs, close):
    {
        DEBUG("received fs.close");
            payload_t a0 = payloads[2];
            fat_fs_close(__ch, (ident_t) a0);
            return;
    }
    case SERVICE(fs, read):
    {
        DEBUG("received fs.read");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            fat_fs_read(__ch, (ident_t) a0, (offset_t) a1);
            return;
    }
    case SERVICE(fs, write):
    {
        DEBUG("received fs.write");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            fat_fs_write(__ch, (ident_t) a0, (offset_t) a1, (void *) a2);
            return;
    }
    case SERVICE(pager, fill):
    {
        DEBUG("received pager.fill");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            fat_pager_fill(__ch, (ident_t) a0, (offset_t) a1, (size_t) a2);
            return;
    }
    }

    WARN("unsupported service: interface=%d, serivce=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
