#include "gc.h"
#include "malloc.h"

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
}
