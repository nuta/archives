#ifndef __ENA_GC_H__
#define __ENA_GC_H__

#include "internal.h"
#include "eval.h"

struct ena_object *ena_alloc_object(struct ena_vm *vm, ena_value_type_t type);

static inline bool ena_is_in_heap(struct ena_vm *vm, ena_value_t value) {
    uintptr_t start = (uintptr_t) vm->value_pool;
    size_t pool_size = ENA_MAX_NUM_VALUES * sizeof(struct ena_object);
    return start <= value && value <= start + pool_size;
}

#endif
