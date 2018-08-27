#ifndef __ENA_GC_H__
#define __ENA_GC_H__

#include "internal.h"
#include "eval.h"

ena_value_t ena_share(ena_value_t value);
void ena_delete(ena_value_t value);
void ena_share_scope(struct ena_scope *scope);
void ena_delete_scope(struct ena_scope *scope);
void ena_delete_table(struct ena_hash_table *table);

#endif
