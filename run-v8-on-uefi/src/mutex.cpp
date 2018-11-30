#include <efijs/mutex.h>
#include <efijs/thread.h>

using namespace efijs::mutex;
using namespace efijs::thread;

bool Mutex::do_try_lock() {
    thread_id_t current = getThreadId();
    if (__sync_bool_compare_and_swap(&state, 0, 1)) {
        owner = current;
        depth = 1;
        return true;
    }
    if (recursive && owner == current) {
        // This recursive mutex is locked by the current thread itself.
        depth++;
        return true;
    }
       
    return false;
}


void Mutex::mark_as_recursive() {
    owner = getThreadId();
    recursive = true;
}


void Mutex::lock() {
    while (!do_try_lock()); 
}


void Mutex::unlock() {
    depth--;
    if (recursive && depth > 1) {
        EFIJS_ASSERT_NEQ(owner, 0);
        return;
    }
    state = 0;
}


bool Mutex::try_lock() {
    return do_try_lock();
}
