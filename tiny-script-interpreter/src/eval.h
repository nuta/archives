#ifndef __ENA_EVAL_H__
#define __ENA_EVAL_H__

#include "internal.h"

/// 1 if `v` is true in `if (v)` or 0 otherwise.
#define ENA_TEST(v) ((v) == ENA_TRUE)
#define ENA_OBJ2VALUE(obj) ((ena_value_t) (obj))

#define PUSH_UNWIND_POINT() \
    do { \
        struct ena_unwind_point *unwind_point = ena_malloc(sizeof(*unwind_point)); \
        unwind_point->prev = vm->current_unwind_point; \
        vm->current_unwind_point = unwind_point; \
    } while(0)
#define EXEC_UNWIND_POINT() ena_setjmp(vm->current_unwind_point->jmpbuf)
#define UNWIND_UNWIND_POINT(unwind_type) ena_longjmp(vm->current_unwind_point->jmpbuf, unwind_type); UNREACHABLE
#define POP_UNWIND_POINT() \
    do { \
        struct ena_unwind_point *current_unwind_point = vm->current_unwind_point; \
        if (!current_unwind_point) { \
            BUG("pop unwind_point in the top level"); \
        } \
        vm->current_unwind_point = current_unwind_point->prev; \
        ena_free(current_unwind_point); \
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

struct ena_userdata {
    struct ena_object_header header;
    void *data;
    void (*free)(struct ena_vm *vm, void *data);
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
        struct ena_userdata userdata;
    };
};

typedef enum {
    // Don't use this type; `0' is bad for the second argument of longjmp.
    ENA_UNWIND_UNUSED_TYPE = 0,

    ENA_UNWIND_RETURN = 1,
    ENA_UNWIND_BREAK = 2,
    ENA_UNWIND_CONTINUE = 3,
} ena_unwind_type_t;

struct ena_unwind_point {
    struct ena_unwind_point *prev;
    ena_jmpbuf jmpbuf;
    ena_value_t ret_value;
};

struct ena_frame {
    struct ena_frame *prev;
    struct ena_func *func;
    struct ena_node *called_from;
};

#define PUSH_FRAME(func_object) do { \
        struct ena_frame *new_frame = ena_malloc(sizeof(*new_frame)); \
        new_frame->func = func_object; \
        new_frame->called_from = vm->current_node; \
        new_frame->prev = vm->current_frame; \
        vm->current_frame = new_frame; \
    } while(0)

#define POP_FRAME() do { \
        if (!vm->current_frame) { \
            break; \
        } \
        struct ena_frame *prev = vm->current_frame->prev; \
        ena_free(vm->current_frame); \
        vm->current_frame = prev; \
    } while(0)

// Assuming that ena_alloc_object() always returns an aligned address.
#define IS_SMALLINT(value)  (((value) & 1) != 0)
#define INT2SMALLINT(value) (((value) << 1) | 1)
#define SMALLINT2INT(value) ((value) >> 1)

// Assuming that sizeof(ena_value_t) > sizeof(uint16_t).
// TODO: support negative integers.
#define SMALL_INT_MAX ((int) sizeof(uint16_t))

static inline int ena_to_int(struct ena_vm *vm, ena_value_t value) {
    if (IS_SMALLINT(value)) {
        return SMALLINT2INT(value);
    } else if (ena_get_type(vm, value)) {
        return ((struct ena_int *) value)->value;
    } else {
        // XXX: Unreachable
        return 0;
    }
}

#endif
