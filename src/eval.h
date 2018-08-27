#ifndef __ENA_EVAL_H__
#define __ENA_EVAL_H__

#include "internal.h"
#include "hash.h"

struct ena_object {
    uint32_t type; // enum ena_value_type
    uint32_t refcount;
};

struct ena_int {
    struct ena_object header;
    int value;
};

#define STRING_FLAG_STORED 0
#define STRING_FLAG_IDENT  1
struct ena_string {
    struct ena_object header;
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
    struct ena_object header;
    uint32_t flags;
    ena_ident_t name;
    struct ena_scope *scope;
    union {
        // flags & FUNC_FLAGS_NATIVE != 0
        struct {
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
    struct ena_object header;
    /// Method table. (ena_ident_t -> ena_value_t)
    struct ena_hash_table methods;
};

struct ena_instance {
    struct ena_object header;
    struct ena_class *cls;
    /// Method table. (ena_ident_t -> ena_value_t)
    struct ena_hash_table props;
};

struct ena_list {
    struct ena_object header;
    ena_value_t *elems;
    size_t num_elems;
};

static inline enum ena_value_type ena_get_type_from_object(struct ena_object *obj) {
    return obj->type;
}

/// 1 if `v` is true in `if (v)` or 0 otherwise.
#define ENA_TEST(v) ((v) == ENA_TRUE)

// Object-related macros.
#define ENA_OBJ2VALUE(obj) ((ena_value_t) (obj))

// Undefined
#define ENA_IS_UNDEFINED(value) ((value) == ENA_UNDEFINED)

// Null
#define ENA_IS_NULL(value) ((value) == ENA_NULL)

// Bool
#define ENA_IS_BOOL(value) ((value) == ENA_TRUE || (value) == ENA_FALSE)

// Int
#define ENA_IS_INT(value) \
    (ena_get_type_from_object((struct ena_object *) value) == ENA_T_INT)

// String
#define ENA_IS_STRING(value) \
    (ena_get_type_from_object((struct ena_object *) value) == ENA_T_STRING)

// Func
#define ENA_IS_FUNC(value) \
    (ena_get_type_from_object((struct ena_object *) value) == ENA_T_FUNC)

// List
#define ENA_IS_LIST(value) \
    (ena_get_type_from_object((struct ena_object *) value) == ENA_T_LIST)

// Class
#define ENA_IS_CLASS(value) \
    (ena_get_type_from_object((struct ena_object *) value) == ENA_T_CLASS)
#define ENA_IS_INSTANCE(value) \
    (ena_get_type_from_object((struct ena_object *) value) == ENA_T_INSTANCE)

#define SCOPE_FLAG_MODULE 1
#define SCOPE_FLAG_LOCALS 0
struct ena_scope {
    uint32_t flags;
    /// ena_ident_t -> ena_value_t
    struct ena_hash_table vars;
    /// For closures.
    int refcount;
    /// The parent scope.
    struct ena_scope *parent;
};

struct ena_module {
    /// This must be the first element so that we can cast module into a
    /// struct ena_scope.
    struct ena_scope scope;
};

enum ena_unwind_type {
    // Don't use this type; `0' is bad for the second argument of longjmp.
    ENA_UNWIND_UNUSED_TYPE = 0,

    ENA_UNWIND_RETURN = 1,
    ENA_UNWIND_BREAK = 2,
    ENA_UNWIND_CONTINUE = 3,
};

struct ena_savepoint {
    struct ena_savepoint *prev;
    ena_jmpbuf jmpbuf;
    ena_value_t ret_value;
};

#endif
