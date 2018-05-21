#include <assert.h>
#include <arch.h>
#include <mutex.h>


STATIC_ASSERT(sizeof(mutex_t) == sizeof(tid_t),
              "sizeof(mutex_t) should equals to sizeof(tid_t)");


void mutex_init(mutex_t *lock) {

    mutex_unlock(lock);
}


void mutex_lock(mutex_t *lock) {

  retry:
    while (*lock);

    *lock = arch_get_current_thread();

    if (*lock != arch_get_current_thread())
        goto retry;
}


bool mutex_try_lock(mutex_t *lock) {

    if (*lock)
        return false;

    *lock = arch_get_current_thread();
    return (*lock == arch_get_current_thread());
}


void mutex_unlock(mutex_t *lock) {

    *lock = 0;
}
