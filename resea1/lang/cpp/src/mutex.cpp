#include "cpp.h"
#include <resea.h>


/**
 *  Initializes a mutex
 *
 *  @param[out] m     The mutex to be initialized.
 *  @param[in]  init  The initial state of the mutex.
 *
 */
void init_mutex (mutex_t *m, int init) {

    *m = init;
}


/**
 *  Acquires a lock from a mutex
 *
 *  @param[in] m  The mutex.
 *
 */
void lock_mutex (mutex_t *m) {

    while(!COMPARE_AND_SWAP(m, MUTEX_UNLOCKED, MUTEX_LOCKED));
}


/**
 *  Releases a lock from a mutex
 *
 *  @param[in] m  The mutex.
 *
 */
void unlock_mutex (mutex_t *m) {

    *m = MUTEX_UNLOCKED;
}
