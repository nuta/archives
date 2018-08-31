/// @file
/// @brief map object.
#include "map.h"
#include "malloc.h"
#include "eval.h"

static ena_value_t map_get(struct ena_vm *vm, ena_value_t self, ena_value_t *args, int num_args) {
    ena_check_args(vm, "get()", "x", args, num_args);
    struct ena_map *self_map = ena_to_map_object(self);
    ena_value_t key = args[0];

    struct ena_hash_entry *entry = ena_hash_search(&self_map->entries, (void *) key);
    return entry ? (ena_value_t) entry->value : ENA_NULL;
}

struct ena_class *ena_create_map_class(struct ena_vm *vm) {
    ena_value_t cls = ena_create_class(vm);
    ena_define_method(vm, cls, "get", map_get);
    return ena_to_class_object(cls);
}
