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

int ena_memcmp(void *ptr1, const void *ptr2, size_t len) {
    uint8_t *ptr2_p = (uint8_t *) ptr2;
    uint8_t *ptr1_p = (uint8_t *) ptr1;

    while (len > 0) {
        int tmp = *ptr2_p - *ptr1_p;
        if (tmp) {
            return tmp;
        }

        ptr1_p++;
        ptr2_p++;
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
    e = ena_hash_search(&vm->cstr2ident, (void *) str);

    ena_hash_digest_t ident;
    if (e) {
        ident = (ena_ident_t) e->value;
    } else {
        // A new hash entry is created.
        ident = vm->next_ident;

        // The `str` is no longer available after returing from this
        // function. Copy it to store in the `ident2cstr` table.
        char *new_str = ena_strdup(str);
        ena_hash_insert(&vm->cstr2ident, (void *) new_str, (void *) ident);
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
    if (ena_get_type(value) != ENA_T_INT) {
        // Invalid cast: `value` is not an int.
        return NULL;
    }

    return (struct ena_int *) value;
}

struct ena_string *ena_cast_to_string(ena_value_t value) {
    if (ena_get_type(value) != ENA_T_STRING) {
        // Invalid cast: `value` is not a string.
        return NULL;
    }

    return (struct ena_string *) value;
}

void ena_define_var_in(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name, ena_value_t value) {
    if (ena_hash_search_or_insert(&scope->vars, (void *) name, (void *) value)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }
}

void ena_define_var(struct ena_vm *vm, ena_ident_t name, ena_value_t value) {
    ena_define_var_in(vm, vm->current_scope, name, value);
}

ena_value_t get_var_from(struct ena_hash_table *table, ena_ident_t name) {
    struct ena_hash_entry *e = ena_hash_search(table, (void *) name);
    if (e) {
        return (ena_value_t) e->value;
    }

    return ENA_UNDEFINED;
}

struct ena_hash_entry *lookup_var(struct ena_scope *scope, ena_ident_t name) {
    while (scope) {
        struct ena_hash_entry *e = ena_hash_search(&scope->vars, (void *) name);
        if (e) {
            return e;
        }

        scope = scope->parent;
    }

    return NULL;
}

ena_value_t get_var_value(struct ena_scope *scope, ena_ident_t name) {
    struct ena_hash_entry *e = lookup_var(scope, name);
    return e ? ((ena_value_t) e->value) : ENA_UNDEFINED;
}

struct ena_class *ena_create_class(void) {
    struct ena_class *cls = ena_malloc(sizeof(*cls));
    cls->header.type = ENA_T_CLASS;
    cls->header.refcount = 0;
    ena_hash_init_ident_table(&cls->methods);
    return cls;
}

static const char *get_type_name(enum ena_value_type type) {
    switch (type) {
        case ENA_T_INT:
            return "int";
        case ENA_T_STRING:
            return "string";
        case ENA_T_LIST:
            return "list";
        case ENA_T_MAP:
            return "map";
        case ENA_T_BOOL:
            return "bool";
        case ENA_T_FUNC:
            return "func";
        case ENA_T_NULL:
            return "null";
        case ENA_T_CLASS:
            return "class";
        case ENA_T_INSTANCE:
            return "instance";
        case ENA_T_UNDEFINED:
            return "(undefined)";
        default:
            // Must be a bug.
            return "(unknown)";
    }

    return NULL;
}

void ena_check_args(struct ena_vm *vm, const char *name, const char *rule, ena_value_t *args, int num_args) {
    char *r = (char *) rule;
    int arg_index = 0;
    while (*r) {
        enum ena_value_type expected_type;
        switch (*r) {
            case 's':
                expected_type = ENA_T_STRING;
                break;
            case 'x':
                expected_type = ENA_T_ANY;
                break;
        }

        r++;
        int expected_num;
        switch (*r) {
            default:
                expected_num = 1;
        }

        if (num_args < arg_index + expected_num) {
            RUNTIME_ERROR("%s takes at least %d argument (%d given)", name, arg_index + expected_num, num_args);
        }

        enum ena_value_type type = ena_get_type(args[arg_index]);
        if (expected_type != ENA_T_ANY && type != expected_type) {
            RUNTIME_ERROR("%s %d%s argument must be %s (%s given)",
                name,
                arg_index,
                (arg_index == 1) ? "st" : ((arg_index == 2) ? "nd" : ((arg_index ==3) ? "rd" : "th")),
                get_type_name(expected_type), get_type_name(type));
        }

        if (!*r) {
            return;
        }

        r++;
        arg_index++;
    }
}

bool ena_is_equal(ena_value_t v1, ena_value_t v2) {
    if (ena_get_type(v1) != ena_get_type(v2)) {
        return false;
    }

    switch (ena_get_type(v1)) {
        case ENA_T_BOOL:
        case ENA_T_NULL:
            return v1 == v2;
        case ENA_T_INT:
            return ((struct ena_int *) v1)->value - ((struct ena_int *) v2)->value;
        case ENA_T_STRING:
            return ((struct ena_string *) v1)->ident == ((struct ena_string *) v2)->ident;
    }

    return false;
}
