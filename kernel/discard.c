#include <resea.h>
#include "kmalloc.h"
#include "message.h"


result_t discard(void *buffer, size_t size) {

    payload_t *payloads = (payload_t *) buffer;
    size_t payloads_num = size / sizeof(payload_t);
    payload_t header = 0;
    for (int i=0; i < payloads_num; i++) {
        if (IS_PAYLOAD_HEADER_INDEX(i)) {
            header = payloads[i];
        } else {
            int type = GET_PAYLOAD_TYPE(header, i);

            switch (type) {
            case PAYLOAD_POINTER:
                try_kfree((void *) payloads[i]);
                break;
            }
        }
    }

    return OK;
}
