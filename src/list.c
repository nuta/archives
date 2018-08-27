/// @file
/// @brief List object.
#include "list.h"
#include "malloc.h"
#include "eval.h"

ena_value_t list_append(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "append()", "x", args, num_args);
    struct ena_list *self_list = (struct ena_list *) self;
    ena_value_t item = args[0];

    size_t new_num_elems = self_list->num_elems + 1;
    ena_value_t *new_elems = ena_malloc(sizeof(ena_value_t) * new_num_elems);
    ena_memcpy(new_elems, self_list->elems, sizeof(ena_value_t) * self_list->num_elems);
    new_elems[new_num_elems - 1] = item;

    self_list->elems = new_elems;
    self_list->num_elems = new_num_elems;
    return ENA_UNDEFINED;
}

struct ena_class *ena_create_list_class(struct ena_vm *vm) {
    struct ena_class *cls = ena_create_class();
    ena_define_native_method(vm, cls, "append", list_append);
    return cls;
}
