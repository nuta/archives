#include "malloc.h"
#include "eval.h"
#include "gc.h"
#include "internal.h"

static ena_value_t *lookup_var(struct ena_scope *scope, ena_ident_t name) {
    while (scope) {
        struct ena_hash_entry *e = ena_hash_search(&scope->vars, (void *) name);
        if (e) {
            return (ena_value_t *) &e->value;
        }

        scope = scope->parent;
    }

    return NULL;
}

struct ena_scope *ena_create_scope(struct ena_vm *vm, struct ena_scope *parent) {
    // TODO: Allocate a scope by alloca().
    struct ena_scope *scope = (struct ena_scope *) ena_alloc_object(vm, ENA_T_SCOPE);
    scope->parent = parent;
    ena_hash_init_ident_table(&scope->vars);
    return scope;
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
    if (ident == IDENT_ANONYMOUS) {
        return "(anonymous)";
    }

    return (const char *) ena_hash_search(&vm->ident2cstr, (void *) ident)->value;
}

void ena_define_var(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name, ena_value_t value) {
    if (ena_hash_search_or_insert(&scope->vars, (void *) name, (void *) value)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, name));
    }
}

ena_value_t ena_get_var_value (struct ena_scope *scope, ena_ident_t name) {
    ena_value_t *value = lookup_var(scope, name);
    return value ? *value : ENA_UNDEFINED;
}

bool ena_set_var_value(struct ena_scope *scope, ena_ident_t name, ena_value_t new_value) {
    ena_value_t *value = lookup_var(scope, name);
    if (!value) {
        return false;
    }

    *value = new_value;
    return true;
}

static const char *get_type_name(ena_value_type_t type) {
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
        case ENA_T_MODULE:
            return "module";
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
        ena_value_type_t expected_type;
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

        ena_value_type_t type = ena_get_type(args[arg_index]);
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
