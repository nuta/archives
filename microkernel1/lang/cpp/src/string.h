#ifndef __STRING_H__
#define __STRING_H__


#include <stddef.h>

void *memchrseq(const void *p, size_t p_size, const char ch, size_t n);

#ifdef POSIX_HOST

#include_next <string.h>

#else

void strcpy_s(char *dest, size_t dest_size, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
void *memset(void *p, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t size);
int memcmp(const void *s1, const void *s2, size_t n);

#endif

#endif
