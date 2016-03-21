#include <resea.h>
#include <string.h>

#ifndef POSIX_HOST

/**
 *  Returns the length of a string
 *
 *  @param[out] s  The NULL-terminated ASCII string.
 *  @return  The length of the string.
 *
 */
size_t strlen(const char *s) {
    size_t n;

    for (n=0; *s; s++)
        n++;

    return n;
}


/**
 *  Copies a string
 *
 *  @param[out] dest       The destination buffer.
 *  @param[in]  dest_size  The size of `dest`.
 *  @param[in]  src        The string to be copied.
 *
 */
void strcpy_s(char *dest, size_t dest_size, const char *src) {
    size_t i;

    for (i=0; src[i] != '\0' && i < dest_size; i++)
        dest[i] = src[i];

    dest[(i == dest_size)? i-1 : i] = '\0';
}


/**
 *  Compares strings (with the max number of comparsion)
 *
 *  @param[in] s1  The string to be compared.
 *  @param[in] s2  The string to be compared.
 *  @param[in] n   The number of bytes to be compared.
 *  @return  0 if first `n` characters of `s1` and `s2` is identical,
 *           otherwise the difference of the first differing bytes.
 *
 */
int strncmp(const char *s1, const char *s2, size_t n) {

    for (size_t i=0; *s1 && *s2 && *s1 - *s2 == 0 && i < n-1; i++) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}


/**
 *  Compares strings
 *
 *  @param[in] s1  The string to be compared.
 *  @param[in] s2  The string to be compared.
 *  @return  0 if `s1` and `s2` is identical, otherwise
 *           the difference of the first differing bytes.
 *
 */
int strcmp(const char *s1, const char *s2) {

    while (*s1 && *s2 && *s1 - *s2 == 0) {
        s1++;
        s2++;
    }
  
    return *s1 - *s2;
}


/**
 *  Copies a memory block
 *
 *  @param[out] dest  The destination buffer.
 *  @param[in]  src   The source buffer.
 *  @param[in]  n     The number of bytes to be copied.
 *
 */
void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d;
    const uint8_t *s;

    d = (uint8_t *) dest;
    s = (const uint8_t *) src;

    for (size_t i=0; i < n; i++)
        d[i] = s[i];

    return dest;
}


/**
 *  Fills memory block with a byte value
 *
 *  @param[out] p  The memory block to be filled.
 *  @param[in]  c  The byte value.
 *  @param[in]  n  The number of bytes to set.
 *
 */
void *memset(void *p, int c, size_t n) {
    uint8_t *_p;

    _p = (uint8_t *) p;

    while(n > 0) {
        *_p = (uint8_t) c;
        n--;
        _p++;
    }

    return _p;
}


/**
 *  Compares contents of two memory blocks
 *
 *  @param[in] s1  The memory block to be compared.
 *  @param[in] s2  The memory block to be compared.
 *  @param[in] n   The number of bytes to be compared.
 *  @return  0 if `s1` and `s2` is identical, otherwise
 *           the difference of the first differing bytes.
 *
 */
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p, *q;

    p = (const unsigned char *) s1;
    q = (const unsigned char *) s2;

    while(n > 0) {
        if (*p != *q)
            return *p - *q;

        n--;
        p++;
        q++;
    }

    return 0;
}


#endif


/**
 *  Locates a byte sequence in block of memory
 *
 *  @param[in] p       The memory block.
 *  @param[in] p_size  The size of `p`.
 *  @param[in] ch      The contents of byte sequence to be located.
 *  @param[in] n       The size of byte sequence to be located.
 *  @return A pointer to the first occurrence or NULL if it is not
 *          found.
 *
 */
void *memchrseq(const void *p, size_t p_size, const char ch, size_t n) {
    unsigned char *q = (unsigned char *) p;
    size_t remain = n;
    void *st = NULL;

    for (; q + p_size > q; q++) {
        if (*q == ch) {
            if (!st)
                st = q;

            remain--;
            if (remain == 0)
                return st;
        } else {
            q      -= n - remain;
            st      = NULL;
            remain  = n;
        }
    }

    return NULL;
}
