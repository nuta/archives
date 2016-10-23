#ifndef __CPP_STRING_H__
#define __CPP_STRING_H__

#include <types.h>

__EXTERN_C__

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
result_t memcpy_s(void *dest, size_t dest_size, const void *src, size_t size);

__EXTERN_C_END__

#endif
