#ifndef __ENA_UTILS_H__
#define __ENA_UTILS_H__

// The standard libraries we can use are limited to freestanding ones.
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef _MSC_VER
#define UNUSED
#define UNREACHABLE
#else
#define UNUSED __attribute__((unused))
#define UNREACHABLE  __builtin_unreachable()
#endif

/// TODO: implement by ourselves
#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#define ena_memchr memchr
#define ena_snprintf snprintf
#define ENA_ASSERT assert
#define ena_setjmp _setjmp
#define ena_longjmp _longjmp
#define ena_jmpbuf jmp_buf

void *ena_memcpy(void *dst, const void *src, size_t len);
int ena_memcmp(void *ptr1, const void *ptr2, size_t len);
int ena_strcmp(const char *s1, const char *s2);
int ena_strncmp(const char *s1, const char *s2, size_t n);
size_t ena_strlen(const char *str);
int ena_str2int(const char *str);
char *ena_strdup(const char *str);
char *ena_strndup(const char *str, size_t len);

static inline int ena_isdigit(int c) {
    return '0' <= c && c <= '9';
}

static inline int ena_isalpha(int c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

static inline int ena_isupper(int c) {
    return 'A' <= c && c <= 'Z';
}

static inline int ena_isalnum(int c) {
    return ena_isdigit(c) || ena_isalpha(c);
}

static inline bool ena_isascii(char ch) {
    return !(!!(ch & 0x80));
}

#endif
