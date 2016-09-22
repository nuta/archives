#include <queue.h>
#include <mutex.h>
#include <malloc.h>


void queue_put(struct queue *queue, void *value) {

    mutex_lock(&queue->lock);

    struct queue_elem *new_e = malloc(sizeof(struct queue_elem));

    new_e->next  = NULL;
    new_e->value = value;

    if (queue->head) {
        struct queue_elem *e = queue->head;

        while (e->next) {
            e = e->next;
        }

        e->next = new_e;
    } else {
        queue->head = new_e;
    }

    mutex_unlock(&queue->lock);
}


void *queue_get(struct queue *queue) {

    mutex_lock(&queue->lock);

    if (!queue->head) {
        mutex_unlock(&queue->lock);
        return NULL;
    }

    struct queue_elem *head = queue->head;
    void *value = head->value;

    queue->head = head->next;
    free(head);

    mutex_unlock(&queue->lock);
    return value;
}


size_t queue_length(struct queue *queue) {

    mutex_lock(&queue->lock);

    size_t n = 0;
    struct queue_elem *e = queue->head;

    while (e) {
        e = e->next;
        n++;
    }

    mutex_unlock(&queue->lock);
    return n;
}


void queue_init(struct queue *queue) {

    mutex_init(&queue->lock);
    queue->head = NULL;
}
