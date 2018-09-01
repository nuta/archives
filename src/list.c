/// @file
/// @brief List object.
#include "list.h"
#include "malloc.h"
#include "eval.h"

static ena_value_t list_append(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "append()", "x", args, num_args);
    struct ena_list *self_list = ena_to_list_object(vm, self);
    ena_value_t item = args[0];

    size_t new_num_elems = self_list->num_elems + 1;
    ena_value_t *new_elems = ena_malloc(sizeof(ena_value_t) * new_num_elems);
    ena_memcpy(new_elems, self_list->elems, sizeof(ena_value_t) * self_list->num_elems);
    new_elems[new_num_elems - 1] = item;

    self_list->elems = new_elems;
    self_list->num_elems = new_num_elems;
    return ENA_UNDEFINED;
}

static ena_value_t list_prepend(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "prepend()", "x", args, num_args);
    struct ena_list *self_list = ena_to_list_object(vm, self);
    ena_value_t item = args[0];

    size_t new_num_elems = self_list->num_elems + 1;
    ena_value_t *new_elems = ena_malloc(sizeof(ena_value_t) * new_num_elems);
    ena_memcpy(&new_elems[1], self_list->elems, sizeof(ena_value_t) * self_list->num_elems);
    new_elems[0] = item;

    self_list->elems = new_elems;
    self_list->num_elems = new_num_elems;
    return ENA_UNDEFINED;
}

static ena_value_t list_pop(UNUSED struct ena_vm *vm, ena_value_t self, UNUSED ena_value_t *args, UNUSED int num_args) {
    struct ena_list *self_list = ena_to_list_object(vm, self);

    if (self_list->num_elems == 0) {
        return ENA_NULL;
    }

    ena_value_t item = self_list->elems[self_list->num_elems - 1];
    self_list->num_elems--;
    return item;
}

struct ena_class *ena_create_list_class(struct ena_vm *vm) {
    ena_value_t cls = ena_create_class(vm);
    ena_define_method(vm, cls, "append", list_append);
    ena_define_method(vm, cls, "prepend", list_prepend);
    ena_define_method(vm, cls, "pop", list_pop);
    return ena_to_class_object(vm, cls);
}
