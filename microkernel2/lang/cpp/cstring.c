#include <string.h>


// This is for C++ default constructors. Use memcpy_s instead.
void *memcpy(void *dest, const void *src, size_t n) {

    memcpy_s(dest, n, src, n);
    return dest;
}


int memcmp(const void *s1, const void *s2, size_t len) {
    unsigned char *p = (unsigned char *) s1;
    unsigned char *q = (unsigned char *) s2;

    while (len > 0) {
        int diff = *p - *q;
        if (diff != 0)
            return diff;

        p++;
        q++;
        len--;
    }

    return 0;
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
        c++;
    }

    return c;
}


int strncmp(const char *s1, const char *s2, size_t len) {

    while (*s1 && *s2 && len > 0) {
        int diff = *s1 - *s2;
        if (diff != 0)
            return diff;

        s1++;
        s2++;
        len--;
    }

    return (len > 0) ? *s1 - *s2 : 0;
}


int strcmp(const char *s1, const char *s2) {

    return strncmp(s1, s2, strlen(s1));
}


// I think this should be in string.h. Just because.
long int atol(const char *s) {

    long int x = 0;
    while (*s && '0' <= *s && *s <= '9') {
        x = (x * 10) + (*s - '0');
        s++;
    }

    return x;
}
