use core::cell::UnsafeCell;
use core::mem::offset_of;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::handle::HandleRight;
use ftl_types::poll::Readiness;
use ftl_utils::static_assert;

use crate::arch;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::poll::Poll;
use crate::poll::PollEmitter;
use crate::process::IDLE_PROCESS;
use crate::process::Process;
use crate::scheduler::GLOBAL_SCHEDULER;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;

/// A continuation of a thread.
///
/// This is the kernel-side equivalent of `Future`. It's named `Promise`
/// not to confuse you with `Future` in the standard library.
///
/// Each variant of this enum represents why a thread is blocked, and
/// contains the complete state to make progress.
pub enum Promise {
    PollWait(SharedRef<Poll>),
}

impl Promise {
    /// Tries to make progress and hopefully unblock the thread.
    ///
    /// Returns `Some(sysret)` if the thread has been unblocked.
    fn poll(&mut self) -> Option<Result<usize, ErrorCode>> {
        let result = match self {
            Promise::PollWait(poll) => {
                let event = poll.try_wait()?;
                Ok(event.as_raw())
            }
        };

        Some(result)
    }
}

enum ThreadState {
    Idle,
    Runnable,
    Blocked(Promise),
    Exited,
    Destroyed,
}

struct Mutable {
    state: ThreadState,
}

/// A thread object.
#[repr(C)]
pub struct Thread {
    arch: arch::Thread,
    process: SharedRef<Process>,
    mutable: SpinLock<Mutable>,
    emitter: PollEmitter,
}

static_assert!(offset_of!(Thread, arch) == 0);

impl Thread {
    pub fn new(
        process: SharedRef<Process>,
        pc: usize,
        sp: usize,
        arg: usize,
    ) -> Result<SharedRef<Thread>, ErrorCode> {
        SharedRef::new(Thread {
            arch: arch::Thread::new(pc, sp, arg),
            process,
            mutable: SpinLock::new(Mutable {
                state: ThreadState::Runnable,
            }),
            emitter: PollEmitter::new(Readiness::empty()),
        })
    }

    pub fn new_idle() -> Result<SharedRef<Thread>, ErrorCode> {
        SharedRef::new(Thread {
            arch: arch::Thread::new_idle(),
            process: IDLE_PROCESS.clone(),
            mutable: SpinLock::new(Mutable {
                state: ThreadState::Idle,
            }),
            emitter: PollEmitter::new(Readiness::empty()),
        })
    }

    pub fn process(&self) -> &SharedRef<Process> {
        &self.process
    }

    pub fn unblock(self: &SharedRef<Self>) {
        let mutable = self.mutable.lock();
        debug_assert!(matches!(mutable.state, ThreadState::Blocked(_)));

        // Try Promise::poll() later.
        GLOBAL_SCHEDULER.push(self.clone());
    }

    pub fn block(&self, blocked_by: Promise) {
        let mut mutable = self.mutable.lock();
        mutable.state = ThreadState::Blocked(blocked_by);
    }

    pub fn exit(&self) {
        let mut mutable = self.mutable.lock();
        mutable.state = ThreadState::Exited;
        self.emitter.set(Readiness::PEER_CLOSED);
    }
}

impl Handleable for Thread {
    fn emitter(&self) -> Option<&PollEmitter> {
        Some(&self.emitter)
    }

    fn close(&self) {
        let mut mutable = self.mutable.lock();
        if matches!(mutable.state, ThreadState::Exited | ThreadState::Destroyed) {
            return;
        }

        mutable.state = ThreadState::Destroyed;
        self.emitter.set(Readiness::PEER_CLOSED);
        debug_warn!("thread destroyed");
    }
}

pub fn sys_thread_spawn(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
) -> Result<SyscallResult, ErrorCode> {
    let process_id = HandleId::from_usize(a0)?;
    let pc = a1;
    let sp = a2;
    let arg = a3;

    let mut handle_table = current.process().handles().lock();
    let process = handle_table
        .get::<Process>(process_id)?
        .authorize(HandleRight::WRITE)?;

    let new_thread = Thread::new(process, pc, sp, arg)?;
    let handle_id = handle_table.insert(Handle::new(new_thread.clone(), HandleRight::ALL))?;
    GLOBAL_SCHEDULER.push(new_thread.clone());

    Ok(SyscallResult::Return(handle_id.as_usize()))
}

/// Terminates the current thread.
pub fn sys_thread_exit(current: &SharedRef<Thread>) -> Result<SyscallResult, ErrorCode> {
    current.exit();
    Ok(SyscallResult::Exit)
}

/// The current thread running on a CPU.
///
/// This is an replacement for `SharedRef<Thread>` in the cpu-local storage
/// to presreve the following properties:
///
/// - The thread running on a CPU will never be dropped. This struct owns a
///   reference count of the thread (`SharedRef`).
///
/// - The offset 0 of this struct is the same as `*mut Thread`. This allows
///   easy access to the `Thread` struct from assembly code.
#[repr(C)]
pub struct CurrentThread {
    ptr: UnsafeCell<*mut Thread>,
}

static_assert!(offset_of!(CurrentThread, ptr) == 0);

impl CurrentThread {
    pub fn new(idle_thread: &SharedRef<Thread>) -> Self {
        Self {
            ptr: UnsafeCell::new(idle_thread.clone().into_raw() as *mut Thread),
        }
    }

    pub fn set_syscall_result(&self, result: Result<usize, ErrorCode>) {
        unsafe {
            let ptr = *self.ptr.get();
            (*ptr).arch.set_syscall_result(result);
        }
    }

    fn update(&self, new: SharedRef<Thread>) {
        let new_ptr = new.into_raw() as *mut Thread;
        let old_ptr = unsafe { core::ptr::replace(self.ptr.get(), new_ptr) };

        // Decrement the ref count of the current thread.
        drop(unsafe { SharedRef::from_raw(old_ptr) });
    }

    fn arch_thread_ptr(&self) -> *mut arch::Thread {
        // SAFETY: static_assert!(offset_of!(Thread, arch) == 0) guarantees arch::Thread
        //         is the first field at the offset 0.
        unsafe { *self.ptr.get() as *mut arch::Thread }
    }

    pub fn as_sharedref(&self) -> SharedRef<Thread> {
        unsafe {
            let ptr = *self.ptr.get();
            let temp = SharedRef::from_raw(ptr as *const Thread);
            let cloned = temp.clone();
            core::mem::forget(temp);
            cloned
        }
    }
}

impl core::ops::Deref for CurrentThread {
    type Target = Thread;

    fn deref(&self) -> &Self::Target {
        unsafe {
            let ptr: *mut Thread = *self.ptr.get();
            &*ptr
        }
    }
}

/// Switches to the thread execution: save the current thread, picks the next
/// thread to run, and restores the next thread's context.
pub fn return_to_user() -> ! {
    if let Some(next_thread) = schedule() {
        arch::switch_into_thread(next_thread);
    } else {
        arch::idle();
    }
}

/// Picks the next thread to run.
fn schedule() -> Option<*mut arch::Thread> {
    let cpuvar = arch::get_cpuvar();
    let current_thread = &cpuvar.current_thread;
    if matches!(current_thread.mutable.lock().state, ThreadState::Runnable) {
        // The current thread is still runnable. Keep running it.
        return Some(current_thread.arch_thread_ptr());
    }

    while let Some(next) = GLOBAL_SCHEDULER.pop_next() {
        // Try unblocking the next thread.
        let mut mutable = next.mutable.lock();
        match &mut mutable.state {
            ThreadState::Runnable => {
                // The thread is already runnable.
                drop(mutable);
                current_thread.update(next);
            }
            ThreadState::Blocked(promise) => {
                if let Some(result) = promise.poll() {
                    // We've just unblocked this thread. Mark it as runnable and
                    // switch to it.
                    mutable.state = ThreadState::Runnable;
                    drop(mutable);
                    current_thread.update(next);
                    current_thread.set_syscall_result(result);
                } else {
                    // The promise is not yet fulfilled. Keep this thread blocked
                    // and pop the next thread from the runqueue.
                    continue;
                }
            }
            ThreadState::Destroyed => {
                // The thread has been destroyed, and was still in the runqueue.
                continue;
            }
            ThreadState::Idle | ThreadState::Exited => {
                // Idle/exited threads should not be in the runqueue.
                unreachable!();
            }
        }

        // Switch to the next thread's address space.
        current_thread.process().isolation().vmspace().switch();
        return Some(current_thread.arch_thread_ptr());
    }

    // No threads to run.
    None
}
