/// @file
/// @brief A string (in UTF-8) object.
#include "string.h"
#include "malloc.h"
#include "eval.h"
#include "gc.h"
#include "utils.h"

/// Searches haystack for needle.
/// @arg start The needle is searched from haystack{start].
/// @arg haystack The UTF-8 string.
/// @arg haystack_size The size in bytes of haystack.
/// @arg needle The UTF-8 string.
/// @arg needle_size The size in bytes of needle.
/// @returns The beginning of the first occurence in haystack if it is found
///          or NULL otherwise.
static char *search(const char *haystack, size_t start, size_t haystack_size, const char *needle, size_t needle_size) {
    if (needle_size == 0) {
        return NULL;
    }

    size_t pos = start;
    for (;;) {
        void *begin;
        if ((begin = ena_memchr(&haystack[pos], needle[0], haystack_size - pos)) == NULL) {
            break;
        }

        // Found the first character.
        size_t offset = (uintptr_t) begin - (uintptr_t) haystack;
        size_t remaining = haystack_size - offset;
        if (remaining < needle_size) {
            return NULL;
        }

        if (ena_memcmp(begin, needle, needle_size) == 0) {
            return begin;
        }

        pos += offset;
    }

    return NULL;
}

/// Handles escape sequences and copy the string.
/// @arg str The string terminated by NUL.
/// @arg size The size of the string in bytes.
/// @returns The NUL-terminated newly allocated string.
static const char *handle_escape_sequence(const char *str, size_t size) {
    char *str2 = ena_malloc(size + 1);
    size_t str2_i = 0;
    size_t str_i = 0;
    while (str_i < size) {
        if (str[str_i] == '\\') {
            ENA_ASSERT(str_i + 1 <= size);
            switch (str[str_i + 1]) {
                case 't':
                    str2[str2_i] = '\t';
                    str_i += 2; // skip backslash and `t'
                    str2_i++;
                    goto next_char;
                case 'n':
                    str2[str2_i] = '\n';
                    str_i += 2; // skip backslash and `n'
                    str2_i++;
                    goto next_char;
                case '"':
                    str2[str2_i] = '"';
                    str_i += 2; // skip backslash and `"'
                    str2_i++;
                    goto next_char;
            }
        } else {
            str2[str2_i] = str[str_i];
            str2_i++;
            str_i++;
        }

next_char:;
    }

    str2[str2_i] = '\0';
    return str2;
}

/// Validate if str is a valid UTF-8 sequence.
/// @arg str The byte sequence. It may contain NUL character.
/// @arg size The size of str in bytes.
/// @returns `true` if str is a valid UTF-8 sequence.
bool ena_utf8_validate(const char *str, size_t size) {
    size_t i = 0;
    for (;;) {
        if (i >= size) {
            break;
        }

        if (ena_isascii(str[i])) {
            i++;
        } else {
            // TODO: Conform https://tools.ietf.org/html/rfc3629
            i++;
        }
    }

    return true;
}

/// Returns the length of UTF-8 string.
/// @arg str The byte sequence. It may contain NUL character.
/// @arg size The size of str in bytes.
/// @warning `str` must be a valid UTF-8 sequence.
/// @returns The length.
size_t ena_utf8_strlen(const char *str, size_t size) {
    size_t i = 0;
    size_t len = 0;
    for (;;) {
        if (i >= size) {
            break;
        }

        int char_len;
        if ((str[i] & 0xf8) == 0xf0) {
            char_len = 4;
        } else if ((str[i] & 0xf0) == 0xe0) {
            char_len = 3;
        } else if ((str[i] & 0xe0) == 0xc0) {
            char_len = 2;
        } else {
            char_len = 1;
        }

        i += char_len;
        len++;
    }

    return len;
}

/// Returns the character at the specified index.
/// @arg str The byte sequence.
/// @arg size The size of str in bytes.
/// @arg index The index.
/// @note O(n)
/// @warning `str` must be a valid UTF-8 sequence.
/// @returns The character at `index`.
uint32_t ena_utf8_char_at(const char *str, size_t size, size_t index) {
    size_t i = 0;
    for (;;) {
        if (i >= size) {
            break;
        }

        int char_len;
        if ((str[i] & 0xf8) == 0xf0) {
            char_len = 4;
            if (index == 0) {
                ENA_ASSERT(i + 3 < size);
                return   ((str[i + 0] & 0x07) << 18)
                       | ((str[i + 1] & 0x3f) << 12)
                       | ((str[i + 2] & 0x3f) << 6)
                       | (str[i + 3] & 0x3f);
            }
        } else if ((str[i] & 0xf0) == 0xe0) {
            char_len = 3;
            if (index == 0) {
                ENA_ASSERT(i + 2 < size);
                return   ((str[i + 0] & 0x0f) << 12)
                       | ((str[i + 1] & 0x3f) << 6)
                       | (str[i + 2] & 0x3f);
            }
        } else if ((str[i] & 0xe0) == 0xc0) {
            char_len = 2;
            if (index == 0) {
                ENA_ASSERT(i + 1 < size);
                return   ((str[i + 0] & 0x1f) << 6)
                       | (str[i + 1] & 0x3f);
            }
        } else {
            char_len = 1;
            if (index == 0) {
                return str[i];
            }
        }

        i += char_len;
        index--;
    }

    return FFFD_CHAR;
}

// @note Assumes that `str` does not contain NUL (verified by utf8_validate()).
ena_value_t ena_create_string(struct ena_vm *vm, const char *str, size_t size) {
    struct ena_string *obj = (struct ena_string *) ena_alloc_object(vm, ENA_T_STRING);
    obj->flags = STRING_FLAG_IDENT;

    if (!ena_utf8_validate(str, size)) {
        RUNTIME_ERROR("Invalid utf-8 byte sequence.");
    }

    const char *buf = handle_escape_sequence(str, size);
    obj->ident = ena_cstr2ident(vm, buf);
    obj->str = ena_ident2cstr(vm, obj->ident);
    obj->size_in_bytes = size;
    ena_free((void *) buf);
    return ENA_OBJ2VALUE(obj);
}

static ena_value_t concat(struct ena_vm *vm,
                          const char *str1, size_t str1_size,
                          const char *str2, size_t str2_size) {
    size_t new_str_size = str1_size + str2_size;
    char *new_str = ena_malloc(new_str_size + 1);
    ena_memcpy(new_str, str1, str1_size);
    ena_memcpy(&new_str[str1_size], str2, str2_size);
    new_str[new_str_size] = '\0';
    return ena_create_string(vm, new_str, new_str_size);
}

static ena_value_t string_concat(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "concat()", "s", args, num_args);
    struct ena_string *self_str = (struct ena_string *) self;
    struct ena_string *str = (struct ena_string *) args[0];
    return concat(vm, self_str->str, self_str->size_in_bytes, str->str, str->size_in_bytes);
}

static ena_value_t string_contains(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "contains()", "s", args, num_args);
    struct ena_string *self_str = (struct ena_string *) self;
    struct ena_string *needle = (struct ena_string *) args[0];

    char *pos = search(self_str->str, 0, self_str->size_in_bytes, needle->str, needle->size_in_bytes);
    return (pos == NULL) ? ENA_FALSE : ENA_TRUE;
}

static ena_value_t string_find(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "find()", "s", args, num_args);
    struct ena_string *self_str = (struct ena_string *) self;
    struct ena_string *needle = (struct ena_string *) args[0];

    char *pos = search(self_str->str, 0, self_str->size_in_bytes, needle->str, needle->size_in_bytes);
    return ena_create_int(vm, (pos == NULL) ? -1 : (int) ((uintptr_t) pos - (uintptr_t) self_str->str));
}

static ena_value_t string_replace(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "replace()", "ss", args, num_args);
    struct ena_string *self_str = (struct ena_string *) self;
    struct ena_string *old_str = (struct ena_string *) args[0];
    struct ena_string *new_str = (struct ena_string *) args[1];
    struct ena_string *new_self = (struct ena_string *) ena_create_string(vm, "", 0);

    size_t index = 0;
    for (;;) {
        if (index >= self_str->size_in_bytes) {
            break;
        }

        char *pos = search(self_str->str, index, self_str->size_in_bytes,
            old_str->str, old_str->size_in_bytes);

        if (pos == NULL) {
            break;
        }

        size_t match = (uintptr_t) pos - (uintptr_t) self_str->str;
        // Copy substring before the matched position and append new_str.
        new_self = (struct ena_string *) concat(vm, new_self->str, new_self->size_in_bytes, &self_str->str[index], match - index);
        new_self = (struct ena_string *) concat(vm, new_self->str, new_self->size_in_bytes, new_str->str, new_str->size_in_bytes);
        index = match + old_str->size_in_bytes;
    }

    // Copy trailing substring.
    new_self = (struct ena_string *) concat(vm, new_self->str, new_self->size_in_bytes, &self_str->str[index], self_str->size_in_bytes - index);

    return ENA_OBJ2VALUE(new_self);
}

struct ena_class *ena_create_string_class(struct ena_vm *vm) {
    ena_value_t cls = ena_create_class(vm);
    ena_define_method(vm, cls, "+", string_concat);
    ena_define_method(vm, cls, "concat", string_concat);
    ena_define_method(vm, cls, "contains", string_contains);
    ena_define_method(vm, cls, "replace", string_replace);
    ena_define_method(vm, cls, "find", string_find);
    return ena_to_class_object(vm, cls);
}
