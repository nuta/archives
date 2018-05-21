#include <mutex.h>
#include "list.h"


void insert_into_list(struct list **list, void *e) {

    while (*list) {
        if (*list == (*list)->next) {
            BUG("circular linked list %p", *list);
            return;
        }

        list = &((*list)->next);
    }

    ((struct list *) e)->next = NULL;
    *list = e;
}


void remove_from_list(struct list **list, void *e) {

    while (*list) {
        if (*list == e)
            *list = (*list)->next;

        if (!*list)
            return;

        list = &((*list)->next);
    }
}
