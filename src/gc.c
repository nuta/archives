#include "gc.h"
#include "malloc.h"

/// Increments the reference count.
/// @arg obj The object.
ena_value_t ena_share(ena_value_t value) {
    if (!ENA_IS_IN_HEAP(value)) {
        // Not an object allocated in the heap.
        return value;
    }

    struct ena_object *obj = (struct ena_object *) value;
    obj->refcount++;
    return value;
}

/// Decrements the reference count and delete the object if needed.
/// @arg obj The object.
void ena_delete(ena_value_t value) {
    if (!ENA_IS_IN_HEAP(value)) {
        // Not an object allocated in the heap.
        return;
    }

    struct ena_object *obj = (struct ena_object *) value;
    obj->refcount--;
    if (obj->refcount == 0) {
        ena_free(obj);
    }
}

/// Increments the reference count of scope.
/// @arg scope The scope.
void ena_share_scope(struct ena_scope *scope) {
    scope->refcount++;
}

/// Decrements the reference count of scope.
/// @arg scope The scope.
void ena_delete_scope(struct ena_scope *scope) {
    scope->refcount--;
    if (scope->refcount == 0) {
        ena_delete_table(&scope->vars);
        ena_free(scope);
    }
}

/// Decrements the reference counts in the specified table.
/// @arg table The table which values are ena_value_t.
void ena_delete_table(struct ena_hash_table *table) {
    for (int i = 0; i < table->num_buckets; i++) {
        struct ena_hash_entry *e = table->buckets[i];
        while (e) {
            ena_delete((ena_value_t) e->value);
            e = e->next;
        }
    }

    ena_hash_free_table(table);
}
