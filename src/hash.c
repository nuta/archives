/// @file
/// @brief A chained-with-linked-list fashion hash table implementation.
#include "internal.h"
#include "malloc.h"
#include "hash.h"
#include "api.h"
#include "eval.h"
#include "internal.h"

/// Reduce the number of entries in a bucket.
/// @arg table The hash table.
static void rehash(UNUSED struct ena_hash_table *table) {
    // TODO
}

/// Initializes a hash table.
/// @arg table The hash table.
/// @arg methods The hash methods.
void ena_hash_init_table(UNUSED struct ena_vm *vm, struct ena_hash_table *table, struct ena_hash_methods *methods) {
    table->methods = methods;
    table->num_entries = 0;
    table->num_buckets = INITIAL_NUM_BUCKETS;
    table->buckets = ena_malloc(sizeof(struct ena_hash_entry *) * INITIAL_NUM_BUCKETS);
    for (int i = 0; i < INITIAL_NUM_BUCKETS; i++) {
        table->buckets[i] = NULL;
    }
}

/// Frees a hash table.
/// @arg table The hash table.
void ena_hash_free_table(UNUSED struct ena_vm *vm, struct ena_hash_table *table, void (*free_value)(void *value)) {
    for (int i = 0; i < table->num_buckets; i++) {
        struct ena_hash_entry *e = table->buckets[i];
        while (e) {
            struct ena_hash_entry *next = e->next;
            if (free_value) {
                free_value(e->value);
            }
            ena_free(e);
            e = next;
        }
    }

    ena_free(table->buckets);
}

/// Searches the table for a given key.
/// @arg table The hash table.
/// @arg key The key.
/// @returns The entry if it is found or NULL otherwise.
struct ena_hash_entry *ena_hash_search(struct ena_vm *vm, struct ena_hash_table *table, void *key) {
    ena_hash_digest_t digest = table->methods->hash(vm, key);
    struct ena_hash_entry *entry = table->buckets[digest % table->num_buckets];
    while (entry != NULL) {
        if (ENTRY_EQUALS(vm, table, entry, digest, key)) {
            return entry;
        }

        entry = entry->next;
    }

    return NULL;
}

/// Inserts an entry (allows duplication).
/// @arg table The hash table.
/// @arg key The key.
/// @arg value The value.
/// @note This function inserts a new entry even if there is an entry with the same key.
void ena_hash_insert(struct ena_vm *vm, struct ena_hash_table *table, void *key, void *value) {
    ena_hash_digest_t digest = table->methods->hash(vm, key);
    struct ena_hash_entry **entry = &table->buckets[digest % table->num_buckets];
    for (;;) {
        if (*entry == NULL) {
            // Prepend a new entry to the end of bucket.
            struct ena_hash_entry *new_entry = ena_malloc(sizeof(*new_entry));
            new_entry->key   = key;
            new_entry->value = value;
            new_entry->hash  = table->methods->hash(vm, key);
            new_entry->next  = NULL;
            *entry = new_entry;
            table->num_entries++;

            if (REHASH_NEEDED(table)) {
                // Too many entries. Add some buckets.
                rehash(table);
            }

            break;
        }

        entry = &(*entry)->next;
    }
}

/// Searches for an entry or inserts an new entry if it does not exists.
/// @arg table The hash table.
/// @arg key The key.
/// @arg value The value.
/// @note This function doesn't insert if the table has key present.
/// @returns NULL if successful or an existing entry (with same key) if it already exists.
struct ena_hash_entry *ena_hash_search_or_insert(struct ena_vm *vm, struct ena_hash_table *table, void *key, void *value) {
    struct ena_hash_entry *entry = ena_hash_search(vm, table, key);
    if (entry) {
        return entry;
    }

    ena_hash_insert(vm, table, key, value);
    return NULL;
}

/// Removes an entry from the table.
/// @arg table The hash table.
/// @arg key The key of entry to be removed.
/// @note This function removes the *first* entry. To remove all entries with the same key use
///       ena_hash_remove_all().
/// @returns true if it found and removed an entry with the key or false otherwise.
bool ena_hash_remove(struct ena_vm *vm, struct ena_hash_table *table, void *key) {
    ena_hash_digest_t digest = table->methods->hash(vm, key);
    struct ena_hash_entry *prev = NULL;
    struct ena_hash_entry *current = table->buckets[digest % table->num_buckets];
    while (current != NULL) {
        if (ENTRY_EQUALS(vm, table, current, digest, key)) {
            if (prev) {
                prev->next = current->next;
            }

            ena_free(current);
            table->num_entries--;
            return true;
        }

        prev = current;
        current = current->next;
    }

    return false;
}

/// Removes all entries with given key from the table.
/// @arg table The hash table.
/// @arg key The key of entry to be removed.
void ena_hash_remove_all(struct ena_vm *vm, struct ena_hash_table *table, void *key) {
    while (ena_hash_remove(vm, table, key));
}

void ena_hash_foreach_value(UNUSED struct ena_vm *vm, struct ena_hash_table *table, void (*cb)(struct ena_vm *vm, void *value)) {
    for (int i = 0; i < table->num_buckets; i++) {
        struct ena_hash_entry *e = table->buckets[i];
        while (e) {
            cb(vm, e->value);
            e = e->next;
        }
    }
}

static bool ident_equals(UNUSED struct ena_vm *vm, void *key1, void *key2) {
    return (signed long long) key1 == (signed long long) key2;
}

static ena_hash_digest_t ident_hash(UNUSED struct ena_vm *vm, void *key) {
    return (ena_hash_digest_t) key;
}

static struct ena_hash_methods ident_hash_methods = {
    .equals = ident_equals,
    .hash = ident_hash,
};

/// Initializes a int hash table.
/// @arg table The hash table.
/// @arg methods The hash methods.
void ena_hash_init_ident_table(struct ena_vm *vm, struct ena_hash_table *table) {
    ena_hash_init_table(vm, table, &ident_hash_methods);
}

static bool cstr_equals(UNUSED struct ena_vm *vm, void *key1, void *key2) {
    return !ena_strcmp(key1, key2);
}

static ena_hash_digest_t cstr_hash(UNUSED struct ena_vm *vm, void *key) {
    unsigned char *str = (unsigned char *) key;

    ena_hash_digest_t digest = 0;
    while (*str != '\0') {
        digest += *str;
        str++;
    }

    return digest;
}

static struct ena_hash_methods cstr_hash_methods = {
    .equals = cstr_equals,
    .hash = cstr_hash,
};

/// Initializes a C-string (ASCII characters terminated by NUL) hash table.
/// @arg table The hash table.
/// @arg methods The hash methods.
void ena_hash_init_cstr_table(struct ena_vm *vm, struct ena_hash_table *table) {
    ena_hash_init_table(vm, table, &cstr_hash_methods);
}

static bool value_equals(struct ena_vm *vm, void *key1, void *key2) {
    return ena_is_equal(vm, (ena_value_t) key1, (ena_value_t) key2);
}

static ena_hash_digest_t value_hash(struct ena_vm *vm, void *key) {
    switch (ena_get_type(vm, (ena_value_t) key)) {
        case ENA_T_INT:
            return ena_to_int_object(vm, (ena_value_t) key)->value;
        case ENA_T_STRING:
            return ena_to_string_object(vm, (ena_value_t) key)->size_in_bytes;
        default:
            return 0;
    }
}

static struct ena_hash_methods value_hash_methods = {
    .equals = value_equals,
    .hash = value_hash,
};

/// Initializes a value hash table.
/// @arg table The hash table.
/// @arg methods The hash methods.
void ena_hash_init_value_table(struct ena_vm *vm, struct ena_hash_table *table) {
    ena_hash_init_table(vm, table, &value_hash_methods);
}

/// Print all entries in a hash<ena_ident_t, ena_value_t>.
void ena_hash_dump_ident_value_table(struct ena_vm *vm, struct ena_hash_table *table) {
    int num_entries = 0;
    for (int i = 0; i < table->num_buckets; i++) {
        struct ena_hash_entry *e = table->buckets[i];
        while (e) {
            char buf[32];
            ena_ident_t key = (ena_ident_t) e->key;
            ena_value_t value = (ena_value_t) e->value;
            ena_stringify(vm, (char *) &buf, sizeof(buf), value);
            DEBUG("%s -> %s", ena_ident2cstr(vm, key), buf);
            num_entries++;
            e = e->next;
        }
    }

    DEBUG("%d entries in total", num_entries);
}
