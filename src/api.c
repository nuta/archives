/// @file
/// @brief Public APIs.

#include "api.h"
#include "parser.h"
#include "hash.h"
#include "malloc.h"
#include "eval.h"
#include "builtins.h"
#include "string.h"
#include "list.h"
#include "map.h"
#include "gc.h"

void ena_stringify(char *buf, size_t buf_len, ena_value_t value) {
    switch (ena_get_type(value)) {
        case ENA_T_INT:
            ena_snprintf(buf, buf_len, "int(%d)", ena_to_int_object(value)->value);
            break;
        case ENA_T_STRING:
            ena_snprintf(buf, buf_len, "string(%s)", ena_to_string_object(value)->str);
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
        case ENA_T_MODULE:
            ena_snprintf(buf, buf_len, "module");
            break;
        case ENA_T_UNDEFINED:
            ena_snprintf(buf, buf_len, "(undefined)");
            break;
        default:
            ena_snprintf(buf, buf_len, "(invalid)");
            break;
    }
}

ena_value_type_t ena_get_type(ena_value_t v) {
    if (v == ENA_NULL) {
        return ENA_T_NULL;
    }
    if (v == ENA_TRUE || v == ENA_FALSE) {
        return ENA_T_BOOL;
    }

    if (ENA_IS_IN_HEAP(v)) {
        return ((struct ena_object *) v)->type;
    }

    return ENA_T_UNDEFINED;
}

ena_value_t ena_create_bool(int condition) {
    return (condition) ? ENA_TRUE : ENA_FALSE;
}

ena_value_t ena_create_int(int value) {
    struct ena_int *obj = (struct ena_int *) ena_malloc(sizeof(*obj));
    obj->header.type = ENA_T_INT;
    obj->header.refcount = 1;
    obj->value = value;
    return ENA_OBJ2VALUE(obj);
}

ena_value_t ena_create_class(void) {
    struct ena_class *cls = ena_malloc(sizeof(*cls));
    cls->header.type = ENA_T_CLASS;
    cls->header.refcount = 0;
    ena_hash_init_ident_table(&cls->methods);
    return ENA_OBJ2VALUE(cls);
}

ena_error_type_t ena_get_error_type(struct ena_vm *vm) {
    return vm->error.type;
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
    vm->main_module = NULL;
    vm->error.type = ENA_ERROR_NONE;
    ena_hash_init_ident_table(&vm->ident2cstr);
    ena_hash_init_cstr_table(&vm->cstr2ident);
    vm->int_class = ena_create_int_class(vm);
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

    ena_hash_free_values(&vm->ident2cstr);
    ena_hash_free_table(&vm->ident2cstr);
    // Key strings of `vm->cstr2ident` are freed by freeing
    // ident2cstr values; they are identical pointers.
    ena_hash_free_table(&vm->cstr2ident);
    ena_destroy_object((struct ena_object *) vm->main_module);
    ena_destroy_object((struct ena_object *) vm->int_class);
    ena_destroy_object((struct ena_object *) vm->string_class);
    ena_destroy_object((struct ena_object *) vm->list_class);
    ena_destroy_object((struct ena_object *) vm->map_class);

    ena_free(vm);
}

void ena_define_method(struct ena_vm *vm, ena_value_t cls, const char *name, ena_native_method_t method) {
    ena_ident_t ident = ena_cstr2ident(vm, name);
    struct ena_func *func = (struct ena_func *) ena_malloc(sizeof(*func));
    func->header.type = ENA_T_FUNC;
    func->header.refcount = 1;
    func->name = ident;
    func->stmts = NULL;
    func->native_method = method;
    func->scope = vm->current_scope;
    func->flags = FUNC_FLAGS_NATIVE | FUNC_FLAGS_METHOD;
    if (ena_hash_search_or_insert(&ena_to_class_object(cls)->methods, (void *) ident, (void *) func)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, ident));
    }
}
