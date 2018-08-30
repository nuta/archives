#ifndef __API_H__
#define __API_H__

#include "utils.h"

// We use only lower 32-bit and MSB.
typedef uintptr_t ena_value_t;

typedef enum {
    ENA_T_UNDEFINED, // used internally
    ENA_T_NULL,
    ENA_T_INT,
    ENA_T_STRING,
    ENA_T_LIST,
    ENA_T_MAP,
    ENA_T_BOOL,
    ENA_T_FUNC,
    ENA_T_CLASS,
    ENA_T_INSTANCE,
    ENA_T_MODULE,
    ENA_T_ANY,
} ena_value_type_t;

#define ENA_UNDEFINED 0
#define ENA_NULL      (1 << 1)
#define ENA_TRUE      (2 << 1)
#define ENA_FALSE     (3 << 1)

typedef enum {
    ENA_ERROR_NONE,
    ENA_ERROR_NOT_YET_IMPLEMENTED,
    ENA_ERROR_INVALID_SYNTAX,
    ENA_ERROR_BUG,
    ENA_ERROR_TYPE,
    ENA_ERROR_RUNTIME,
} ena_error_type_t;

struct ena_vm;
typedef ena_value_t (*ena_native_method_t)(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args);
typedef ena_value_t (*ena_native_func_t)(struct ena_vm *vm, ena_value_t *args, int num_args);

/// An ena interpreter instance.
struct ena_vm *ena_create_vm();
void ena_destroy_vm(struct ena_vm *vm);
bool ena_eval(struct ena_vm *vm, ena_value_t module, char *script);
ena_error_type_t ena_get_error_type(struct ena_vm *vm);
const char *ena_get_error_cstr(struct ena_vm *vm);

ena_value_type_t ena_get_type(ena_value_t value);
void ena_stringify(char *buf, size_t buf_len, ena_value_t value);

/// To value.
ena_value_t ena_create_int(int value);
ena_value_t ena_create_string(struct ena_vm *vm, const char *str, size_t size);
ena_value_t ena_create_bool(int condition);
ena_value_t ena_create_func(ena_native_func_t native_func);
ena_value_t ena_create_class(void);
void ena_define_method(struct ena_vm *vm, ena_value_t cls, const char *name, ena_native_method_t method);
ena_value_t ena_create_module(void);
void ena_add_to_module(struct ena_vm *vm, ena_value_t module, const char *name, ena_value_t value);
bool ena_is_equal(ena_value_t v1, ena_value_t v2);

#endif
