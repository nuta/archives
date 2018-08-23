#include "malloc.h"
#include "eval.h"
#include "internal.h"

/// Converts an ASCII string into an integer. Supports hexadecimal string
/// (prefixed by 0x) and decimal string.
int ena_str2int(const char *str) {
    char *s = (char *) str;
    int base = 10;
    size_t i;
    if (str[0] != '\0' && str[1] != '\0' && str[0] == '0') {
        switch (str[1]) {
            case 'x':
                /* 0x123, 0xabc, ... */
                base = 16;
                i = 2; // Remove `0x' prefix.
                break;
            default:
                // Invalid string.
                return 0;
        }
    } else {
        i = 0;
    }

    int r = 0;
    while (str[i] != '\0') {
        char ch = s[i];
        int value;
        if (ena_isalpha(ch)) {
            value = (ena_isupper(ch) ? ch - 'A' : ch - 'a') + 10;
        } else {
            value = ch - '0';
        }

        r = r * base + value;
        i++;
    }

    return r;
}

void *ena_memcpy(void *dst, const void *src, size_t len) {
    uint8_t *dst_p = dst;
    uint8_t *src_p = (uint8_t *) src;

    while (len > 0) {
        *dst_p = *src_p;
        dst_p++;
        src_p++;
        len--;
    }

    return dst;
}

int ena_memcmp(void *dst, const void *src, size_t len) {
    uint8_t *dst_p = dst;
    uint8_t *src_p = (uint8_t *) src;

    while (len > 0) {
        int tmp = *dst_p - *src_p;
        if (tmp) {
            return tmp;
        }

        dst_p++;
        src_p++;
        len--;
    }

    return 0;
}

int ena_strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s2 != '\0') {
        int tmp = *s1 - *s2;
        if (tmp) {
            return tmp;
        }

        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int ena_strncmp(const char *s1, const char *s2, size_t n) {
    while (*s1 != '\0' && *s2 != '\0') {
        int tmp = *s1 - *s2;
        if (tmp) {
            return tmp;
        }

        n--;
        if (n == 0) {
            break;
        }

        s1++;
        s2++;
    }

    return *s1 - *s2;
}

size_t ena_strlen(const char *str) {
    size_t len = 0;
    while (*str != '\0') {
        str++;
        len++;
    }

    return len;
}

char *ena_strdup(const char *str) {
    size_t len = ena_strlen(str);
    char *new_str = ena_malloc(len + 1);
    ena_memcpy(new_str, str, len);
    new_str[len] = '\0';
    return new_str;
}

char *ena_strndup(const char *str, size_t len) {
    size_t str_len = ena_strlen(str);
    size_t actual_len = (str_len < len) ? str_len : len;
    char *new_str = ena_malloc(actual_len + 1);
    ena_memcpy(new_str, str, actual_len);
    new_str[len] = '\0';
    return new_str;
}

/// Get the ident associated with a C string.
/// @arg vm  The VM.
/// @arg str The string.
/// @retruns The ident associated with the `str`.
ena_ident_t ena_cstr2ident(struct ena_vm *vm, const char *str) {
    struct ena_hash_entry *e;
    e = ena_hash_search_or_insert(&vm->cstr2ident, (void *) str, (void *) vm->next_ident);

    ena_hash_digest_t ident;
    if (e) {
        ident = (ena_ident_t) e->value;
    } else {
        // A new hash entry is created.
        ident = vm->next_ident;

        // The `str` is no longer available after returing from this
        // function. Copy it to store in the `ident2cstr` table.
        char *new_str = ena_strdup(str);
        ena_hash_insert(&vm->ident2cstr, (void *) ident, (void *) new_str);
        vm->next_ident++;
    }

    return ident;
}

/// Get a pointer to C string associated with given ident.
/// @arg vm  The VM.
/// @arg ident The ident.
/// @retruns The string associated with `ident` if successful or NUL otherwise.
const char *ena_ident2cstr(struct ena_vm *vm, ena_ident_t ident) {
    return (const char *) ena_hash_search(&vm->ident2cstr, (void *) ident)->value;
}


void init_scope(struct ena_scope *scope, struct ena_scope *parent);
struct ena_module *ena_create_module(void) {
    struct ena_module *module = ena_malloc(sizeof(*module));
    init_scope(&module->scope, NULL);
    module->scope.flags = SCOPE_FLAG_MODULE;
    return module;
}

struct ena_int *ena_cast_to_int(ena_value_t value) {
    if (!ENA_IS_TYPE(value, ENA_T_INT)) {
        // Invalid cast: `value` is not an int.
        return NULL;
    }

    return (struct ena_int *) value;
}

void ena_define_var_in(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name, ena_value_t value) {
    if (ena_hash_search_or_insert(&scope->vars, (void *) name, (void *) value)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }
}

void ena_define_var(struct ena_vm *vm, ena_ident_t name, ena_value_t value) {
    ena_define_var_in(vm, vm->current, name, value);
}

void ena_define_func(struct ena_vm *vm, struct ena_hash_table *table, ena_ident_t name, struct ena_node *param_names,  struct ena_node *stmts, int type) {
    struct ena_func *obj = (struct ena_func *) ena_malloc(sizeof(*obj));
    obj->header.type = ENA_T_FUNC;
    obj->header.refcount = 1;
    obj->flags |= type;
    obj->name = name;
    obj->param_names = param_names;
    obj->stmts = stmts;

    if (ena_hash_search_or_insert(table, (void *) name, (void *) obj)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }
}

ena_value_t get_var_from(struct ena_hash_table *table, ena_ident_t name) {
    struct ena_hash_entry *e = ena_hash_search(table, (void *) name);
    if (e) {
        return (ena_value_t) e->value;
    }

    return ENA_UNDEFINED;
}

ena_value_t lookup_var(struct ena_scope *scope, ena_ident_t name) {
    while (scope) {
        ena_value_t value = get_var_from(&scope->vars, name);
        if (value != ENA_UNDEFINED) {
            return value;
        }

        scope = scope->parent;
    }

    return ENA_UNDEFINED;
}

void ena_assign_to_var(struct ena_vm *vm, struct ena_hash_table *table, ena_ident_t name, ena_value_t value, bool allow_undefined) {
    struct ena_hash_entry *e;
    if (allow_undefined) {
        e = ena_hash_search_or_insert(table, (void *) name, (void *) value);
        if (!e) {
            // Successfully defined a new variable.
            return;
        }
    } else {
        e = ena_hash_search(table, (void *) name);
        if (!e) {
            RUNTIME_ERROR("%s is not defined", ena_ident2cstr(vm, name));
        }
    }

    e->value = (void *) value;
}
