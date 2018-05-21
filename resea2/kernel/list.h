#ifndef __KERNEL_LIST_H__
#define __KERNEL_LIST_H__

#include <resea.h>

struct list {
    struct list *next;
};

void insert_into_list(struct list **list, void *e);
void remove_from_list(struct list **list, void *e);

#endif
