#include "list.h"


void list_init(struct list **list) {
    *list = NULL;
}


/* Caller have to lock a mutex. */
void list_append(struct list **list, void *e) {

    while (*list) {
        if (*list == (*list)->next) {
            /* Circular linked list. */
            return;
        }

        list = &((*list)->next);
    }

    ((struct list *) e)->next = NULL;
    *list = e;
}


/* Caller have to lock a mutex. */
void *list_pop(struct list **list) {

    if (*list) {
        struct list *e = *list;
        *list = e->next;
        return e;
    }

    return NULL;
}


/* Caller have to lock a mutex. */
void list_remove(struct list **list, void *e) {

    while (*list) {
        if (*list == e) {
            *list = (*list)->next;
        }

        if (!*list) {
            return;
        }

        list = &((*list)->next);
    }
}
