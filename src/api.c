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
        return ((struct ena_object *) v)->header.flags & OBJECT_FLAG_TYPE_MASK;
    }

    return ENA_T_UNDEFINED;
}

ena_value_t ena_create_bool(UNUSED struct ena_vm *vm, int condition) {
    return (condition) ? ENA_TRUE : ENA_FALSE;
}

ena_value_t ena_create_int(struct ena_vm *vm, int value) {
    struct ena_int *obj = (struct ena_int *) ena_alloc_object(vm, ENA_T_INT);
    obj->value = value;
    return ENA_OBJ2VALUE(obj);
}

ena_value_t ena_create_class(struct ena_vm *vm) {
    struct ena_class *cls = (struct ena_class *) ena_alloc_object(vm, ENA_T_CLASS);
    ena_hash_init_ident_table(&cls->methods);
    return ENA_OBJ2VALUE(cls);
}

static ena_value_t print_func_handler(UNUSED struct ena_vm *vm, ena_value_t *args, int num_args) {
    for (int i = 0; i < num_args; i++) {
        ena_value_t value = args[i];
        switch (ena_get_type(value)) {
            case ENA_T_INT:
                printf("%d ", ena_to_int_object(value)->value);
                break;
            case ENA_T_STRING:
                printf("%s ", ena_to_string_object(value)->str);
                break;
            case ENA_T_BOOL:
                printf("%s ", value == ENA_TRUE ? "true" : "false");
                break;
            case ENA_T_NULL:
                printf("null ");
                break;
            default:;
                printf("(unprintable) ");
        }
    }

    printf("\n");
    return ENA_NULL;
}

ena_value_t ena_create_module(struct ena_vm *vm) {
    struct ena_module *module = (struct ena_module *) ena_alloc_object(vm, ENA_T_MODULE);
    module->scope = ena_create_scope(vm, NULL);
    module->next = NULL;

#ifndef ENA_NO_PRINT
    ena_value_t print_func = ena_create_func(vm, print_func_handler);
    ena_define_var(vm, module->scope, ena_cstr2ident(vm, "print"), print_func);
#endif

    return ENA_OBJ2VALUE(module);
}

ena_error_type_t ena_get_error_type(struct ena_vm *vm) {
    return vm->error.type;
}

const char *ena_get_error_cstr(struct ena_vm *vm) {
    return vm->error.str;
}

struct ena_vm *ena_create_vm() {
    struct ena_vm *vm = ena_malloc(sizeof(*vm));
    vm->next_ident = IDENT_START;
    vm->ast_list = NULL;
    vm->self = ENA_UNDEFINED;
    vm->current_class = NULL;
    vm->current_savepoint = NULL;
    vm->modules = NULL;
    vm->error.type = ENA_ERROR_NONE;
    vm->stack_end = 0;
    vm->value_pool = ena_malloc(sizeof(struct ena_object) * ENA_MAX_NUM_VALUES);
    ena_memset(vm->value_pool, 0, sizeof(struct ena_object) * ENA_MAX_NUM_VALUES);
    vm->num_free = ENA_MAX_NUM_VALUES;
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

    // Key strings of `vm->cstr2ident` are freed by freeing
    // ident2cstr values; they are identical pointers.
    ena_hash_free_table(&vm->ident2cstr, ena_free);
    ena_hash_free_table(&vm->cstr2ident, NULL);
    ena_free(vm->value_pool);
    ena_free(vm);
}

void ena_define_method(struct ena_vm *vm, ena_value_t cls, const char *name, ena_native_method_t native_method) {
    ena_ident_t ident = ena_cstr2ident(vm, name);
    struct ena_func *func = (struct ena_func *) ena_alloc_object(vm, ENA_T_FUNC);
    func->name = ident;
    func->stmts = NULL;
    func->native_method = native_method;
    func->scope = vm->current_scope;
    func->flags = FUNC_FLAGS_NATIVE | FUNC_FLAGS_METHOD;
    if (ena_hash_search_or_insert(&ena_to_class_object(cls)->methods, (void *) ident, (void *) func)) {
        RUNTIME_ERROR("%s is already defined", ena_ident2cstr(vm, ident));
    }
}

ena_value_t ena_create_func(struct ena_vm *vm, ena_native_func_t native_func) {
    struct ena_func *func = (struct ena_func *) ena_alloc_object(vm, ENA_T_FUNC);
    func->name = IDENT_ANONYMOUS;
    func->stmts = NULL;
    func->native_func = native_func;
    func->scope = NULL;
    func->flags = FUNC_FLAGS_NATIVE;
    return ENA_OBJ2VALUE(func);
}

void ena_register_module(struct ena_vm *vm, UNUSED const char *name, ena_value_t module) {
    struct ena_module **entry = &vm->modules;
    for (;;) {
        if (*entry == NULL) {
            *entry = ena_to_module_object(module);
            return;
        }

        entry = &(*entry)->next;
    }
}

void ena_add_to_module(struct ena_vm *vm, ena_value_t module, const char *name, ena_value_t value) {
    struct ena_module *m = ena_to_module_object(module);
    ena_define_var(vm, m->scope, ena_cstr2ident(vm, name), value);
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
            return ena_to_int_object(v1)->value == ena_to_int_object(v2)->value;
        case ENA_T_STRING:
            return ena_to_string_object(v1)->ident == ena_to_string_object(v2)->ident;
        default:;
            /* TODO: BUG() */
    }

    return false;
}
