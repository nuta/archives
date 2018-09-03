#ifndef __ENA_INTERNAL_H__
#define __ENA_INTERNAL_H__

#include "api.h"
#include "hash.h"
#include "lexer.h"

#ifdef ENA_DEBUG_BUILD
// Including non-freestanding libraries are allowed in debug build.
#include <stdio.h>
#define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#define DEBUG_NONL(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#define DEBUG_NONL(fmt, ...)
#endif

#define ENA_PANIC(error_type, fmt, ...) \
    do {\
        vm->error.type = error_type; \
        ena_snprintf( \
            (char *) &vm->error.str, sizeof(vm->error.str), \
            fmt, ## __VA_ARGS__ \
        ); \
        ena_stacktrace(vm); \
        ena_longjmp(vm->panic_jmpbuf, 1); \
    } while (0)
#define BUG(fmt, ...) \
    ENA_PANIC(ENA_ERROR_BUG, "BUG: " fmt, ## __VA_ARGS__)
#define TYPE_ERROR(fmt, ...) \
    ENA_PANIC(ENA_ERROR_TYPE, "Type Error: " fmt, ## __VA_ARGS__)
#define RUNTIME_ERROR(fmt, ...) \
    ENA_PANIC(ENA_ERROR_RUNTIME, "Runtime Error: " fmt, ## __VA_ARGS__)

#define SYNTAX_ERROR(fmt, ...) \
    ENA_PANIC(ENA_ERROR_INVALID_SYNTAX, \
        "%s: line %d: Syntax Error: " fmt, \
        vm->lexer.filepath, vm->lexer.current_line, ## __VA_ARGS__)

#define NOT_YET_IMPLEMENTED() \
    ENA_PANIC( \
        ENA_ERROR_NOT_YET_IMPLEMENTED, \
        "%s:%s():%d: not yet implemented", \
        __FILE__, __func__, __LINE__ \
    )

#define ENA_VALUE2OBJ(type, value) ((struct ena_##type *) (value))

struct ena_error {
    ena_error_type_t type;
    char str[256];
    int line;
};

struct ena_module;
struct ena_scope;
struct ena_unwind_point;
struct ena_class;
struct ena_frame;
struct ena_object;
struct ena_ast;
struct ena_vm {
    struct ena_error error;
    ena_jmpbuf panic_jmpbuf;
    size_t next_ident;
    struct ena_lexer lexer;
    struct ena_hash_table ident2cstr;
    struct ena_hash_table cstr2ident;
    struct ena_module *modules;
    struct ena_frame *current_frame;
    struct ena_node *current_node;
    struct ena_scope *current_scope;
    struct ena_unwind_point *current_unwind_point;
    struct ena_class *current_class;
    ena_value_t self;
    struct ena_ast *ast_list;
    uintptr_t stack_end;
    struct ena_object *value_pool;
    size_t num_free;
    struct ena_class *int_class;
    struct ena_class *string_class;
    struct ena_class *list_class;
    struct ena_class *map_class;
};

typedef uintptr_t ena_ident_t;
#define IDENT_ANONYMOUS ((ena_ident_t) 1)
#define IDENT_START     ((ena_ident_t) 100)

#define DEFINE_VALUE2OBJ_FUNC(type_name, type_id) \
    static inline struct ena_##type_name *ena_to_##type_name##_object(struct ena_vm *vm, ena_value_t value) { \
        if (ena_get_type(vm, value) != type_id) { \
            DEBUG("value2obj error"); \
            return NULL; \
        } \
     \
        return (struct ena_##type_name *) value; \
    }
DEFINE_VALUE2OBJ_FUNC(string, ENA_T_STRING)
DEFINE_VALUE2OBJ_FUNC(bool, ENA_T_BOOL)
DEFINE_VALUE2OBJ_FUNC(list, ENA_T_LIST)
DEFINE_VALUE2OBJ_FUNC(map, ENA_T_MAP)
DEFINE_VALUE2OBJ_FUNC(instance, ENA_T_INSTANCE)
DEFINE_VALUE2OBJ_FUNC(class, ENA_T_CLASS)
DEFINE_VALUE2OBJ_FUNC(module, ENA_T_MODULE)
DEFINE_VALUE2OBJ_FUNC(userdata, ENA_T_USERDATA)

ena_ident_t ena_cstr2ident(struct ena_vm *vm, const char *str);
const char *ena_ident2cstr(struct ena_vm *vm, ena_ident_t ident);
void ena_define_var(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name, ena_value_t value);
ena_value_t ena_get_var_value(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name);
bool ena_set_var_value(struct ena_vm *vm, struct ena_scope *scope, ena_ident_t name, ena_value_t new_value);
void ena_check_args(struct ena_vm *vm, const char *name, const char *rule, ena_value_t *args, int num_args);
struct ena_scope *ena_create_scope(struct ena_vm *vm, struct ena_scope *parent);
void ena_stacktrace(struct ena_vm *vm);

#endif
