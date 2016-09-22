#ifndef __CPP_QUEUE_H__
#define __CPP_QUEUE_H__

#include <mutex.h>

struct queue_elem {
    void *value;
    struct queue_elem *next;
};

struct queue {
    mutex_t lock;
    struct queue_elem *head;
};


void queue_put(struct queue *queue, void *value);
void *queue_get(struct queue *queue);
size_t queue_length(struct queue *queue);
void queue_init(struct queue *queue);

#endif
