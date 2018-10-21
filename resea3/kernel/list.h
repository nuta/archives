#ifndef __LIST_H__
#define __LIST_H__

#include <kernel/types.h>

struct list {
    struct list *next;
};

#define DEFINE_LIST(name, type)                                     \
    static inline void name##_list_init(type **list) {              \
        list_init((struct list **) list);                           \
    }                                                               \
                                                                    \
    static inline bool name##_list_is_empty(type **list) {          \
        return *list == NULL;                                       \
    }                                                               \
                                                                    \
    static inline void name##_list_append(type **list, type *e) {   \
        list_append((struct list **) list, e);                      \
    }                                                               \
                                                                    \
    static inline type *name##_list_pop(type **list) {              \
        return list_pop((struct list **) list);                     \
    }                                                               \
                                                                    \
    static inline void name##_list_remove(type **list, type *e) {   \
        list_remove((struct list **) list, e);                      \
    }

void list_init(struct list **list);
void list_append(struct list **list, void *e);
void *list_pop(struct list **list);
void list_remove(struct list **list, void *e);

#endif
