#include <string.h>

char *strcpy(char *dst, const char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }

    dst[i] = '\0';
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 - *s2 == 0) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

size_t strlen(const char *s) {

    size_t n = 0;
    while (*s++) {
        n++;
    }

    return n;
}
