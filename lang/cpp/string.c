#include <string.h>


// This is for C++ default constructors. Use memcpy_s instead.
void *memcpy(void *dest, const void *src, size_t n) {

    uint8_t *p = (uint8_t *) dest;
    uint8_t *q = (uint8_t *) src;
    while (n > 0) {
        *p = *q;

        p++;
        q++;
        n--;
    }

    return dest;
}


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
