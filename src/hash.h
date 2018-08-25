#ifndef __ENA_HASH_H__
#define __ENA_HASH_H__

typedef uintptr_t ena_hash_digest_t;

struct ena_hash_methods {
    bool (*equals)(void *key1, void *key2);
    ena_hash_digest_t (*hash)(void *key);
};

struct ena_hash_entry {
    /// The pointer to the next entry in the bucket.
    struct ena_hash_entry *next;
    ena_hash_digest_t hash;
    void *key;
    void *value;
};

struct ena_hash_table {
    struct ena_hash_methods *methods;
    struct ena_hash_entry **buckets;
    int num_buckets;
    int num_entries;
};

void ena_hash_init_table(struct ena_hash_table *table, struct ena_hash_methods *methods);
void ena_hash_free_table(struct ena_hash_table *table);
struct ena_hash_entry *ena_hash_search(struct ena_hash_table *table, void *key);
void ena_hash_insert(struct ena_hash_table *table, void *key, void *value);
struct ena_hash_entry *ena_hash_search_or_insert(struct ena_hash_table *table, void *key, void *value);
bool ena_hash_remove(struct ena_hash_table *table, void *key);
void ena_hash_remove_all(struct ena_hash_table *table, void *key);

void ena_hash_init_ident_table(struct ena_hash_table *table);
void ena_hash_init_cstr_table(struct ena_hash_table *table);

struct ena_vm;
void ena_hash_dump_ident_value_table(struct ena_vm *vm, struct ena_hash_table *table);

#endif
