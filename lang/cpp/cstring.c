#include <string.h>


// This is for C++ default constructors. Use memcpy_s instead.
void *memcpy(void *dest, const void *src, size_t n) {

    memcpy_s(dest, n, src, n);
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


size_t strlen(const char *s) {

    int c = 0;
    while (*s) {
        s++;
    }

    return c;
}

int strcmp(const char *s1, const char *s2) {

    while (*s1 && *s2) {
        int diff = *s1 - *s2;
        if (diff != 0)
            return diff;

        s1++;
        s2++;
    }

    return *s1 - *s2;
}
