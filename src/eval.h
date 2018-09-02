#ifndef __ENA_EVAL_H__
#define __ENA_EVAL_H__

#include "internal.h"

/// 1 if `v` is true in `if (v)` or 0 otherwise.
#define ENA_TEST(v) ((v) == ENA_TRUE)
#define ENA_OBJ2VALUE(obj) ((ena_value_t) (obj))

#define PUSH_UNWIND_POINT() \
    do { \
        struct ena_savepoint *sp = ena_malloc(sizeof(*sp)); \
        sp->prev = vm->current_savepoint; \
        vm->current_savepoint = sp; \
    } while(0)
#define EXEC_UNWIND_POINT() ena_setjmp(vm->current_savepoint->jmpbuf)
#define UNWIND_UNWIND_POINT(unwind_type) ena_longjmp(vm->current_savepoint->jmpbuf, unwind_type); UNREACHABLE
#define POP_UNWIND_POINT() \
    do { \
        struct ena_savepoint *current_sp = vm->current_savepoint; \
        if (!current_sp) { \
            BUG("pop savepoint in the top level"); \
        } \
        vm->current_savepoint = current_sp->prev; \
        ena_free(current_sp); \
    } while(0)

struct ena_object_header {
#define OBJECT_FLAG_TYPE_MASK 0xff
#define OBJECT_FLAG_MARKED    (1 << 16)
    uint32_t flags;
};

struct ena_int {
    struct ena_object_header header;
    int value;
};

#define STRING_FLAG_STORED 0
#define STRING_FLAG_IDENT  1
struct ena_string {
    struct ena_object_header header;
    uint32_t flags;
    const char *str;
    ena_ident_t ident;
    size_t size_in_bytes;
};

#define FUNC_FLAGS_METHOD  (1 << 0)
#define FUNC_FLAGS_FUNC    (0 << 0)
#define FUNC_FLAGS_NATIVE  (1 << 1)
#define FUNC_FLAGS_ENA     (0 << 1)
struct ena_func {
    struct ena_object_header header;
    uint32_t flags;
    ena_ident_t name;
    struct ena_scope *scope;
    union {
        // flags & FUNC_FLAGS_NATIVE != 0
        union {
            ena_native_method_t native_method;
            ena_native_func_t native_func;
        };
        // flags & FUNC_FLAGS_NATIVE == 0
        struct {
            struct ena_node *stmts;
            struct ena_node *param_names;
        };
    };
};

struct ena_class {
    struct ena_object_header header;
    /// Method table. (ena_ident_t -> ena_value_t)
    struct ena_hash_table methods;
};

struct ena_instance {
    struct ena_object_header header;
    struct ena_class *cls;
    struct ena_scope *props;
};

struct ena_list {
    struct ena_object_header header;
    ena_value_t *elems;
    size_t num_elems;
};

struct ena_map {
    struct ena_object_header header;
    struct ena_hash_table entries;
};

struct ena_scope {
    struct ena_object_header header;
    /// ena_ident_t -> ena_value_t
    struct ena_hash_table vars;
    /// The parent scope.
    struct ena_scope *parent;
};

struct ena_module {
    struct ena_object_header header;
    struct ena_module *next;
    struct ena_scope *scope;
};

struct ena_object {
    union {
        struct ena_object_header header;
        struct ena_int i;
        struct ena_string string;
        struct ena_func func;
        struct ena_class cls;
        struct ena_instance instance;
        struct ena_list list;
        struct ena_map map;
        struct ena_module module;
    };
};

typedef enum {
    // Don't use this type; `0' is bad for the second argument of longjmp.
    ENA_UNWIND_UNUSED_TYPE = 0,

    ENA_UNWIND_RETURN = 1,
    ENA_UNWIND_BREAK = 2,
    ENA_UNWIND_CONTINUE = 3,
} ena_unwind_type_t;

struct ena_savepoint {
    struct ena_savepoint *prev;
    ena_jmpbuf jmpbuf;
    ena_value_t ret_value;
};

#endif
