#include "gc.h"
#include "eval.h"
#include "malloc.h"

#ifdef __x86_64__
#define ARCH_NUM_REGS 14

static inline uintptr_t arch_get_stack_bottom() {
    uintptr_t bottom;
    __asm__ __volatile__("movq %%rsp, %0" : "=m"(bottom));
    return bottom;
}

static void arch_load_regs(uintptr_t *regs) {
    __asm__ __volatile__(
        "movq %%rax, %0 \n"
        "movq %%rbx, %1 \n"
        "movq %%rcx, %2 \n"
        "movq %%rdx, %3 \n"
        "movq %%rdi, %4 \n"
        "movq %%rsi, %5 \n"
        "movq %%r8,  %6 \n"
        "movq %%r9,  %7 \n"
        "movq %%r10, %8 \n"
        "movq %%r11, %9 \n"
        "movq %%r12, %10 \n"
        "movq %%r13, %11 \n"
        "movq %%r14, %12 \n"
        "movq %%r15, %13 \n"
    :
    "=m"(regs[0]),
    "=m"(regs[1]),
    "=m"(regs[2]),
    "=m"(regs[3]),
    "=m"(regs[4]),
    "=m"(regs[5]),
    "=m"(regs[6]),
    "=m"(regs[7]),
    "=m"(regs[8]),
    "=m"(regs[9]),
    "=m"(regs[10]),
    "=m"(regs[11]),
    "=m"(regs[12]),
    "=m"(regs[13])
    );
}
#elif __EMSCRIPTEN__
#define ARCH_NUM_REGS 0
static inline uintptr_t arch_get_stack_bottom() {
    return 0;
}

static void arch_load_regs(uintptr_t *regs) {
}
#else
#error "unsupported arch"
#endif


static inline void mark_func(struct ena_func *func);
static inline void mark_class(struct ena_class *cls);
static inline void mark_instance(struct ena_instance *instance);
static inline void mark_map(struct ena_map *map);
static inline void mark_list(struct ena_list *list);
static inline void mark_scope(struct ena_scope *scope);

static inline void mark_object(struct ena_object *obj) {
    obj->header.flags |= OBJECT_FLAG_MARKED;
}

static void mark(ena_value_t value) {
    switch (ena_get_type(value)) {
        case ENA_T_INT:
        case ENA_T_STRING:
            mark_object((struct ena_object *) value);
            break;
        case ENA_T_FUNC:
            mark_func((struct ena_func *) value);
            break;
        case ENA_T_SCOPE:
            mark_scope((struct ena_scope *) value);
            break;
        case ENA_T_CLASS:
            mark_class((struct ena_class *) value);
            break;
        case ENA_T_INSTANCE:
            mark_instance((struct ena_instance *) value);
            break;
        case ENA_T_MAP:
            mark_map((struct ena_map *) value);
            break;
        case ENA_T_LIST:
            mark_list((struct ena_list *) value);
            break;
        default:;
    }
}

static inline void mark_hash_with_value_values(struct ena_hash_table *table) {
    ena_hash_foreach_value(table, (void (*)(void *)) mark);
}

static inline void mark_func(struct ena_func *func) {
    mark_object((struct ena_object *) func);

    if (func->scope) {
        mark_scope(func->scope);
    }
}

static inline void mark_class(struct ena_class *cls) {
    mark_object((struct ena_object *) cls);
    mark_hash_with_value_values(&cls->methods);
}

static inline void mark_list(struct ena_list *list) {
    mark_object((struct ena_object *) list);
    for (size_t i = 0; i < list->num_elems; i++) {
        mark(list->elems[i]);
    }
}

static inline void mark_map(struct ena_map *map) {
    mark_object((struct ena_object *) map);
    mark_hash_with_value_values(&map->entries);
}

static inline void mark_scope(struct ena_scope *scope) {
    if (scope->parent) {
        mark_scope(scope->parent);
    }
    mark_hash_with_value_values(&scope->vars);
}

static inline void mark_instance(struct ena_instance *instance) {
    mark_object((struct ena_object *) instance);
    mark_class(instance->cls);
    mark_scope(instance->props);
}

static inline void mark_module(struct ena_module *module) {
    mark_object((struct ena_object *) module);
    mark_scope(module->scope);
}

static inline void mark_modules(struct ena_vm *vm) {
    for (struct ena_module *module = vm->modules; module != NULL; module = module->next) {
        mark_module(module);
    }
}

static inline void try_mark(struct ena_vm *vm, void *ptr) {
    bool aligned = ((uintptr_t) ptr - (uintptr_t) vm->value_pool) % sizeof(struct ena_object) == 0;
    if (ENA_IS_IN_HEAP(ptr) && aligned) {
        mark((ena_value_t) ptr);
    }
}


static void mark_regs(struct ena_vm *vm) {
    uintptr_t regs[ARCH_NUM_REGS];
    arch_load_regs((uintptr_t *) &regs);
    for (int i = 0; i < ARCH_NUM_REGS; i++) {
        try_mark(vm, (void *) regs[i]);
    }
}

/// @warning Don't create an ena_value_t in this function. The
///          value could be unmarked; __builtin_fram_address(9) returns the
///          content of the stack frame register, not the bottom of stack.
static void mark_stack(struct ena_vm *vm) {
    uintptr_t *ptr = (uintptr_t *) arch_get_stack_bottom();

    // Assumes that stack grows downward.
    while ((uintptr_t) ptr < vm->stack_end) {
        try_mark(vm, ptr);
        ptr++;
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
                obj->header.flags = 0;
                vm->num_free++;
            }
        }
    }
}

void ena_gc(struct ena_vm *vm) {
    mark_modules(vm);
    mark_regs(vm);
    mark_stack(vm);
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
