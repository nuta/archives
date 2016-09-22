#include <string.h>


result_t memcpy_s(void *dest, size_t dest_size, const void *src, size_t size) {

    if (dest_size < size || dest == NULL || src == NULL) {
        return E_BAD_MEMCPY;
    }

    uint8_t *p = (uint8_t *) dest;
    uint8_t *q = (uint8_t *) src;
    while (size > 0) {
        *p = *q;

        p++;
        q++;
        size--;
    }

    return OK;
}
