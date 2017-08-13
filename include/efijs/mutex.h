#pragma once
#include <efijs/assert.h>
#include <efijs/thread.h>

namespace efijs {
namespace mutex {

class Mutex {
private:
    int state;
    efijs::thread::thread_id_t owner;
    bool recursive;
    int depth;

    bool do_try_lock();

public:
    Mutex() : state(0), owner(0), recursive(false), depth(0) {}

    void mark_as_recursive();
    void lock();
    void unlock();
    bool try_lock();
};

} // namespace efijs::mutex
} // namespace efijs