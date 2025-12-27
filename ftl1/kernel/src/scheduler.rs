use alloc::collections::VecDeque;

use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::thread::Thread;

pub static GLOBAL_SCHEDULER: Scheduler = Scheduler::new();

pub struct Scheduler {
    runqueue: SpinLock<VecDeque<SharedRef<Thread>>>,
}

impl Scheduler {
    pub const fn new() -> Scheduler {
        Scheduler {
            runqueue: SpinLock::new(VecDeque::new()),
        }
    }

    pub fn push(&self, new_thread: SharedRef<Thread>) {
        // SAFETY: This should not panic because we've already reserved the
        //         capacity in `try_reserve`.
        self.runqueue.lock().push_back(new_thread);
    }

    pub fn pop_next(&self) -> Option<SharedRef<Thread>> {
        self.runqueue.lock().pop_front()
    }
}
