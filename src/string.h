#ifndef __ENA_STRING_H__
#define __ENA_STRING_H__

#include "internal.h"

#define FFFD_CHAR 0xfffd

bool ena_utf8_validate(const char *str, size_t size);
size_t ena_utf8_strlen(const char *str, size_t size);
uint32_t ena_utf8_char_at(const char *str, size_t size, size_t index);
struct ena_class *ena_create_string_class(struct ena_vm *vm);

#endif
