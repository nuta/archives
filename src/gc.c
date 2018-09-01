#include "gc.h"
#include "eval.h"
#include "malloc.h"
#include "utils.h"

static inline void mark_func(struct ena_vm *vm, struct ena_func *func);
static inline void mark_class(struct ena_vm *vm, struct ena_class *cls);
static inline void mark_instance(struct ena_vm *vm, struct ena_instance *instance);
static inline void mark_map(struct ena_vm *vm, struct ena_map *map);
static inline void mark_list(struct ena_vm *vm, struct ena_list *list);
static inline void mark_scope(struct ena_vm *vm, struct ena_scope *scope);

static inline void mark_object(struct ena_object *obj) {
    obj->header.flags |= OBJECT_FLAG_MARKED;
}

static void mark(struct ena_vm *vm, ena_value_t value) {
     if (ena_is_in_heap(vm, value)) {
        struct ena_object *obj = (struct ena_object *) value;
        if (obj->header.flags & OBJECT_FLAG_MARKED) {
            return;
        }
    }

    switch (ena_get_type(vm, value)) {
        case ENA_T_INT:
        case ENA_T_STRING:
            mark_object((struct ena_object *) value);
            break;
        case ENA_T_FUNC:
            mark_func(vm, (struct ena_func *) value);
            break;
        case ENA_T_SCOPE:
            mark_scope(vm, (struct ena_scope *) value);
            break;
        case ENA_T_CLASS:
            mark_class(vm, (struct ena_class *) value);
            break;
        case ENA_T_INSTANCE:
            mark_instance(vm, (struct ena_instance *) value);
            break;
        case ENA_T_MAP:
            mark_map(vm, (struct ena_map *) value);
            break;
        case ENA_T_LIST:
            mark_list(vm, (struct ena_list *) value);
            break;
        default:;
    }
}

static inline void mark_hash_with_value_values(struct ena_vm *vm, struct ena_hash_table *table) {
    ena_hash_foreach_value(vm, table, (void (*)(struct ena_vm *vm, void *)) mark);
}

static inline void mark_func(struct ena_vm *vm, struct ena_func *func) {
    mark_object((struct ena_object *) func);

    if (func->scope) {
        mark_scope(vm, func->scope);
    }
}

static inline void mark_class(struct ena_vm *vm, struct ena_class *cls) {
    mark_object((struct ena_object *) cls);
    mark_hash_with_value_values(vm, &cls->methods);
}

static inline void mark_list(struct ena_vm *vm, struct ena_list *list) {
    mark_object((struct ena_object *) list);
    for (size_t i = 0; i < list->num_elems; i++) {
        mark(vm, list->elems[i]);
    }
}

static inline void mark_map(struct ena_vm *vm, struct ena_map *map) {
    mark_object((struct ena_object *) map);
    mark_hash_with_value_values(vm, &map->entries);
}

static inline void mark_scope(struct ena_vm *vm, struct ena_scope *scope) {
    mark_object((struct ena_object *) scope);
    mark_hash_with_value_values(vm, &scope->vars);
    if (scope->parent) {
        mark_scope(vm, scope->parent);
    }
}

static inline void mark_instance(struct ena_vm *vm, struct ena_instance *instance) {
    mark_object((struct ena_object *) instance);
    mark_class(vm, instance->cls);
    mark_scope(vm, instance->props);
}

static inline void mark_module(struct ena_vm *vm, struct ena_module *module) {
    mark_object((struct ena_object *) module);
    mark_scope(vm, module->scope);
}

static inline void try_mark(struct ena_vm *vm, ena_value_t ptr) {
    bool aligned = ((uintptr_t) ptr - (uintptr_t) vm->value_pool) % sizeof(struct ena_object) == 0;
    if (ena_is_in_heap(vm, ptr) && aligned) {
        mark(vm, ptr);
    }
}

static void mark_regs(struct ena_vm *vm) {
    uintptr_t regs[ARCH_NUM_REGS];
    arch_load_regs((uintptr_t *) &regs);
    for (int i = 0; i < ARCH_NUM_REGS; i++) {
        try_mark(vm, regs[i]);
    }
}

/// @warning Don't create an ena_value_t in this function. The
///          value could be unmarked; __builtin_fram_address(9) returns the
///          content of the stack frame register, not the bottom of stack.
static void mark_stack(struct ena_vm *vm) {
    uintptr_t *ptr = (uintptr_t *) arch_get_stack_bottom();

    // Assumes that stack grows downward.
    while ((uintptr_t) ptr < vm->stack_end) {
        try_mark(vm, (ena_value_t) *ptr);
        ptr++;
    }
}

static inline void destroy_scope(struct ena_vm *vm, struct ena_scope *scope) {
    ena_hash_free_table(vm, &scope->vars, NULL);
}

static inline void destroy_class(struct ena_vm *vm, struct ena_class *cls) {
    ena_hash_free_table(vm, &cls->methods, NULL);
}

static inline void destroy_map(struct ena_vm *vm, struct ena_map *map) {
    ena_hash_free_table(vm, &map->entries, NULL);
}

static void destroy(struct ena_vm *vm, ena_value_t value) {
    switch (ena_get_type(vm, value)) {
        case ENA_T_SCOPE:
            destroy_scope(vm, (struct ena_scope *) value);
            break;
        case ENA_T_CLASS:
            destroy_class(vm, (struct ena_class *) value);
            break;
        case ENA_T_MAP:
            destroy_map(vm, (struct ena_map *) value);
            break;
        default:;
    }
}

static void sweep(struct ena_vm *vm) {
    for (size_t i = 0; i < ENA_MAX_NUM_VALUES; i++) {
        struct ena_object *obj = &vm->value_pool[i];
        bool in_use = obj->header.flags != 0;
        bool marked = (obj->header.flags & OBJECT_FLAG_MARKED) != 0;
        if (in_use) {
            if (marked) {
                obj->header.flags &= ~OBJECT_FLAG_MARKED;
            } else {
                // The object is no longer referenced. Mark it as unused.
                destroy(vm, ENA_OBJ2VALUE(obj));
                obj->header.flags = 0;
                vm->num_free++;
            }
        }
    }
}

void ena_gc(struct ena_vm *vm) {
#ifdef __EMSCRIPTEN__
    // TODO: We can't implement mark_regs and mark_stack properly I think.
    return;
#endif

    mark_regs(vm);
    mark_stack(vm);

    for (struct ena_module *module = vm->modules; module != NULL; module = module->next) {
        mark_module(vm, module);
    }

    if (vm->current_scope) {
        mark_scope(vm, vm->current_scope);
    }

    if (vm->current_class) {
        mark_class(vm, vm->current_class);
    }

    mark_class(vm, vm->int_class);
    mark_class(vm, vm->string_class);
    mark_class(vm, vm->list_class);
    mark_class(vm, vm->map_class);

    sweep(vm);
}

struct ena_object *ena_alloc_object(struct ena_vm *vm, ena_value_type_t type) {
    if (vm->num_free < ENA_GC_THRESHOLD) {
        ena_gc(vm);
    }

    for (size_t i = 0; i < ENA_MAX_NUM_VALUES; i++) {
        struct ena_object *obj = &vm->value_pool[i];
        if (obj->header.flags == 0) {
            obj->header.flags = type;
            vm->num_free--;
            return obj;
        }
    }

    DEBUG("run out of value pool");
    return NULL;
}
