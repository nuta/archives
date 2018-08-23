/// @file
/// @brief Public APIs.

#include "api.h"
#include "parser.h"
#include "hash.h"
#include "malloc.h"
#include "eval.h"

ena_value_t ena_create_int(int value) {
    struct ena_int *obj = (struct ena_int *) ena_malloc(sizeof(*obj));
    obj->header.type = ENA_T_INT;
    obj->header.refcount = 1;
    obj->value = value;
    return ENA_OBJ2VALUE(obj);
}

void ena_stringify(char *buf, size_t buf_len, ena_value_t value) {
    switch (ena_get_type(value)) {
        case ENA_T_INT:
            ena_snprintf(buf, buf_len, "int(%d)", ena_cast_to_int(value)->value);
            break;
        case ENA_T_BOOL:
            ena_snprintf(buf, buf_len, "bool");
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
        case ENA_T_UNDEFINED:
            ena_snprintf(buf, buf_len, "(undefined)");
            break;
    }
}

enum ena_value_type ena_get_type(ena_value_t v) {
    return (ENA_IS_UNDEFINED(v)) ? ENA_T_UNDEFINED  :
           (ENA_IS_INT(v))       ? ENA_T_INT        :
           (ENA_IS_BOOL(v))      ? ENA_T_BOOL       :
           (ENA_IS_NULL(v))      ? ENA_T_NULL       :
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
    vm->current_instance = NULL;
    vm->current_class = NULL;
    vm->current_savepoint = NULL;
    vm->main_module = ena_create_module();
    vm->error.type = ENA_ERROR_NONE;
    ena_hash_init_ident_table(&vm->ident2cstr);
    ena_hash_init_cstr_table(&vm->cstr2ident);
    return vm;
}
