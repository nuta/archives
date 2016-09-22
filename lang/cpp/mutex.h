#ifndef __CPP_MUTEX_H__
#define __CPP_MUTEX_H__

#include <types.h>
#include <arch_types.h>

void mutex_init(mutex_t *lock);
void mutex_lock(mutex_t *lock);
bool mutex_try_lock(mutex_t *lock);
void mutex_unlock(mutex_t *lock);

#endif

