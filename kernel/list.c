#include <mutex.h>
#include "list.h"


void insert_into_list(struct list **list, void *e) {

    while (*list)
        list = &((*list)->next);

    *list = e;
}


void remove_from_list(struct list **list, void *e) {

    while (*list) {
        if (*list == e)
            *list = (*list)->next;

        list = &((*list)->next);
    }
}
