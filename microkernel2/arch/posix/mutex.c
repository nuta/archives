#include <arch.h>
#include <pthread.h>
#include <mutex.h>


void mutex_init(mutex_t *lock) {

    pthread_mutex_init(lock, NULL);
}


void mutex_lock(mutex_t *lock) {

    pthread_mutex_lock(lock);
}


bool mutex_try_lock(mutex_t *lock) {

    return pthread_mutex_trylock(lock) == 0;
}


void mutex_unlock(mutex_t *lock) {

    pthread_mutex_unlock(lock);
}
