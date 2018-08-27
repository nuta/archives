/// @file
/// @brief Public APIs.

#include "api.h"
#include "parser.h"
#include "hash.h"
#include "malloc.h"
#include "eval.h"
#include "string.h"
#include "list.h"
#include "map.h"

ena_value_t ena_create_int(int value) {
    struct ena_int *obj = (struct ena_int *) ena_malloc(sizeof(*obj));
    obj->header.type = ENA_T_INT;
    obj->header.refcount = 1;
    obj->value = value;
    return ENA_OBJ2VALUE(obj);
}

/// Handles escape sequences and copy the string.
/// @arg str The string terminated by NUL.
/// @arg size The size of the string in bytes.
/// @returns The NUL-terminated newly allocated string.
const char *handle_escape_sequence(const char *str, size_t size) {
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

// @note Assumes that `str` does not contain NUL (verified by utf8_validate()).
ena_value_t ena_create_string(struct ena_vm *vm, const char *str, size_t size) {
    struct ena_string *obj = (struct ena_string *) ena_malloc(sizeof(*obj));
    obj->header.type = ENA_T_STRING;
    obj->header.refcount = 1;
    obj->flags = STRING_FLAG_IDENT;

    if (!utf8_validate(str, size)) {
        RUNTIME_ERROR("Invalid utf-8 byte sequence.");
    }

    const char *buf = handle_escape_sequence(str, size);
    obj->ident = ena_cstr2ident(vm, buf);
    obj->str = ena_ident2cstr(vm, obj->ident);
    obj->size_in_bytes = size;
    ena_free((void *) buf);
    return ENA_OBJ2VALUE(obj);
}

void ena_stringify(char *buf, size_t buf_len, ena_value_t value) {
    switch (ena_get_type(value)) {
        case ENA_T_INT:
            ena_snprintf(buf, buf_len, "int(%d)", ena_cast_to_int(value)->value);
            break;
        case ENA_T_STRING:
            ena_snprintf(buf, buf_len, "string(%s)", ena_cast_to_string(value)->str);
            break;
        case ENA_T_BOOL:
            ena_snprintf(buf, buf_len, "bool(%s)", value == ENA_TRUE ? "true" : "false");
            break;
        case ENA_T_FUNC:
            ena_snprintf(buf, buf_len, "func");
            break;
        case ENA_T_NULL:
            ena_snprintf(buf, buf_len, "null");
            break;
        case ENA_T_CLASS:
            ena_snprintf(buf, buf_len, "class");
            break;
        case ENA_T_INSTANCE:
            ena_snprintf(buf, buf_len, "instance");
            break;
        case ENA_T_LIST:
            ena_snprintf(buf, buf_len, "list");
            break;
        case ENA_T_MAP:
            ena_snprintf(buf, buf_len, "map");
            break;
        case ENA_T_UNDEFINED:
            ena_snprintf(buf, buf_len, "(undefined)");
            break;
    }
}

enum ena_value_type ena_get_type(ena_value_t v) {
    return (ENA_IS_UNDEFINED(v)) ? ENA_T_UNDEFINED  :
           (ENA_IS_NULL(v))      ? ENA_T_NULL       :
           (ENA_IS_BOOL(v))      ? ENA_T_BOOL       :
           (ENA_IS_INT(v))       ? ENA_T_INT        :
           (ENA_IS_STRING(v))    ? ENA_T_STRING     :
           (ENA_IS_LIST(v))      ? ENA_T_LIST       :
           (ENA_IS_MAP(v))       ? ENA_T_MAP        :
           (ENA_IS_FUNC(v))      ? ENA_T_FUNC       :
           (ENA_IS_CLASS(v))     ? ENA_T_CLASS      :
           (ENA_IS_INSTANCE(v))  ? ENA_T_INSTANCE   :
           ENA_T_UNDEFINED;
}

ena_value_t ena_create_bool(int condition) {
    return (condition) ? ENA_TRUE : ENA_FALSE;
}

const char *ena_get_error_cstr(struct ena_vm *vm) {
    return vm->error.str;
}

struct ena_vm *ena_create_vm() {
    struct ena_vm *vm = ena_malloc(sizeof(*vm));
    vm->next_ident = 1;
    vm->ast_list = NULL;
    vm->self = ENA_UNDEFINED;
    vm->current_class = NULL;
    vm->current_savepoint = NULL;
    vm->main_module = ena_create_module();
    vm->error.type = ENA_ERROR_NONE;
    ena_hash_init_ident_table(&vm->ident2cstr);
    ena_hash_init_cstr_table(&vm->cstr2ident);
    vm->string_class = ena_create_string_class(vm);
    vm->list_class = ena_create_list_class(vm);
    vm->map_class = ena_create_map_class(vm);
    return vm;
}

void ena_destroy_vm(struct ena_vm *vm) {
    struct ena_ast *ast = vm->ast_list;
    while (ast != NULL) {
        struct ena_ast *next = ast->next;
        ena_destroy_ast(ast);
        ast = next;
    }

    ena_free(vm);
}

void ena_define_native_method(struct ena_vm *vm, struct ena_class *cls, const char *name, ena_native_method_t method) {
    ena_ident_t ident = ena_cstr2ident(vm, name);
    struct ena_func *func = (struct ena_func *) ena_malloc(sizeof(*func));
    func->header.type = ENA_T_FUNC;
    func->header.refcount = 1;
    func->name = ident;
    func->stmts = NULL;
    func->native_method = method;
    func->scope = vm->current_scope;
    func->flags = FUNC_FLAGS_NATIVE | FUNC_FLAGS_METHOD;
    if (ena_hash_search_or_insert(&cls->methods, (void *) ident, (void *) func)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, ident));
    }
}
