//! A event-driven polling mechanism, similar to Linux's `epoll(2)`.
//!
//! # Locking
//!
//! Since [`Poll`] and [`Listener`] reference each other, you need to be careful
//! about deadlocks. The rule is: do not hold both locks at the same time.
use alloc::collections::BTreeMap;
use alloc::collections::VecDeque;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::handle::HandleRight;
use ftl_types::poll::PollEvent;
use ftl_types::poll::Readiness;

use crate::handle::AnyHandle;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::spinlock::SpinLockGuard;
use crate::syscall::SyscallResult;
use crate::thread::Promise;
use crate::thread::Thread;

#[derive(PartialEq, Eq, PartialOrd, Ord)]
struct ListenerKey(usize);

impl ListenerKey {
    pub fn new<T>(sref: &SharedRef<T>) -> Self {
        // `listeners` keeps a SharedRef<T> reference, so it's safe to use
        // the pointer as its unique identifier.
        Self(sref.as_ptr() as usize)
    }
}

struct Listener {
    poll: SharedRef<Poll>,
    handle_id: HandleId,
    interests: Readiness,
}

struct EmitterInner {
    latest: Readiness,
    listeners: BTreeMap<ListenerKey, Listener>,
}

impl EmitterInner {
    fn notify_all(&self) {
        for listener in self.listeners.values() {
            let events = listener.interests & self.latest;
            if !events.is_empty() {
                listener.poll.notify(listener.handle_id, events);
            }
        }
    }
}

pub struct PollEmitter {
    inner: SpinLock<EmitterInner>,
}

impl PollEmitter {
    pub fn new(initial: Readiness) -> Self {
        Self {
            inner: SpinLock::new(EmitterInner {
                latest: initial,
                listeners: BTreeMap::new(),
            }),
        }
    }

    pub fn set(&self, mask: Readiness) {
        let mut inner = self.inner.lock();
        inner.latest |= mask;
        inner.notify_all();
    }

    pub fn clear(&self, mask: Readiness) {
        let mut inner = self.inner.lock();
        inner.latest &= !mask;
        inner.notify_all();
    }

    fn add_listener(&self, listener: Listener) {
        let key = ListenerKey::new(&listener.poll);
        let mut inner = self.inner.lock();

        // Compare with the latest readiness.
        let events = listener.interests & inner.latest;
        if !events.is_empty() {
            listener.poll.notify(listener.handle_id, events);
        }

        inner.listeners.insert(key, listener);
    }

    fn remove_listener(&self, poll: &SharedRef<Poll>) {
        let key = ListenerKey::new(poll);
        let mut inner = self.inner.lock();
        inner.listeners.remove(&key);
    }

    pub fn update_listener(&self, poll: &SharedRef<Poll>, or_mask: Readiness, and_mask: Readiness) {
        let key = ListenerKey::new(poll);
        let mut inner = self.inner.lock();

        let latest = inner.latest;
        let Some(listener) = inner.listeners.get_mut(&key) else {
            return;
        };

        listener.interests &= and_mask;
        listener.interests |= or_mask;

        // Compare with the updated interests.
        let events = listener.interests & latest;
        if !events.is_empty() {
            listener.poll.notify(listener.handle_id, events);
        }
    }
}

struct Object {
    handle: AnyHandle,
    ready: Readiness,
}

struct Mutable {
    waiters: VecDeque<SharedRef<Thread>>,
    objects: BTreeMap<HandleId, Object>,
    ready_queue: VecDeque<HandleId>,
}

enum TryWaitResult<'a> {
    Ready(PollEvent),
    None(SpinLockGuard<'a, Mutable>),
}

pub struct Poll {
    mutable: SpinLock<Mutable>,
}

impl Poll {
    pub fn new() -> Result<SharedRef<Self>, ErrorCode> {
        SharedRef::new(Poll {
            mutable: SpinLock::new(Mutable {
                waiters: VecDeque::new(),
                objects: BTreeMap::new(),
                ready_queue: VecDeque::new(),
            }),
        })
    }

    pub fn add(
        self: &SharedRef<Self>,
        id: HandleId,
        handle: AnyHandle,
        interests: Readiness,
    ) -> Result<(), ErrorCode> {
        let Some(emitter) = handle.emitter() else {
            return Err(ErrorCode::NotSupported);
        };

        {
            let mut mutable = self.mutable.lock();
            if mutable.objects.contains_key(&id) {
                return Err(ErrorCode::AlreadyExists);
            }

            mutable.objects.insert(
                id,
                Object {
                    handle: handle.clone(),
                    ready: Readiness::empty(),
                },
            );
        }

        emitter.add_listener(Listener {
            poll: self.clone(),
            handle_id: id,
            interests,
        });
        Ok(())
    }

    pub fn update(
        self: &SharedRef<Self>,
        id: HandleId,
        or_mask: Readiness,
        and_mask: Readiness,
    ) -> Result<(), ErrorCode> {
        let handle = {
            let mutable = self.mutable.lock();
            let Some(object) = mutable.objects.get(&id) else {
                return Err(ErrorCode::NotFound);
            };

            object.handle.clone()
        };

        // SAFETY: Poll::add guarantees that the handle has an emitter.
        let emitter = handle.emitter().unwrap();

        emitter.update_listener(self, or_mask, and_mask);
        Ok(())
    }

    pub fn remove(self: &SharedRef<Self>, id: HandleId) -> Result<(), ErrorCode> {
        let handle = {
            let mut mutable = self.mutable.lock();
            let Some(object) = mutable.objects.remove(&id) else {
                return Err(ErrorCode::NotFound);
            };

            object.handle.clone()
        };

        // SAFETY: Poll::add guarantees that the handle has an emitter.
        let emitter = handle.emitter().unwrap();

        emitter.remove_listener(self);
        Ok(())
    }

    fn do_try_wait(self: &SharedRef<Self>) -> TryWaitResult<'_> {
        let mut mutable = self.mutable.lock();
        while let Some(ready_id) = mutable.ready_queue.pop_front() {
            let Some(object) = mutable.objects.get_mut(&ready_id) else {
                // The object has been removed after the object became ready.
                continue;
            };

            // Read the ready events and clear them.
            let readiness = object.ready;
            debug_assert!(!readiness.is_empty());
            object.ready = Readiness::empty();
            let ev = PollEvent::new(ready_id, readiness);

            // Drop the mutable lock to avoid deadlocks.
            let handle = object.handle.clone();
            drop(mutable);

            // SAFETY: Poll::add guarantees that the handle has an emitter.
            let emitter = handle.emitter().unwrap();

            if handle.is_edge_triggered() {
                // Clear the emitter to deliver the event only once.
                emitter.clear(readiness);
            }

            // emitter.deactivate_listener(self);
            return TryWaitResult::Ready(ev);
        }

        TryWaitResult::None(mutable)
    }

    pub fn try_wait(self: &SharedRef<Self>) -> Option<PollEvent> {
        match self.do_try_wait() {
            TryWaitResult::Ready(ev) => Some(ev),
            TryWaitResult::None(_mutable) => None,
        }
    }

    pub fn wait(
        self: &SharedRef<Self>,
        current: &SharedRef<Thread>,
    ) -> Result<SyscallResult, ErrorCode> {
        match self.do_try_wait() {
            TryWaitResult::Ready(ev) => Ok(SyscallResult::Return(ev.as_raw())),
            TryWaitResult::None(mut mutable) => {
                // No ready objects. Block the thread.
                mutable.waiters.push_back(current.clone());
                Ok(SyscallResult::Block(Promise::PollWait(self.clone())))
            }
        }
    }

    fn notify(self: &SharedRef<Self>, id: HandleId, events: Readiness) {
        let waiter = {
            let mut mutable = self.mutable.lock();
            let Some(object) = mutable.objects.get_mut(&id) else {
                return;
            };

            // Update the ready events, and enqueue the object to the ready queue
            // if it's not already in it.
            let was_empty = object.ready.is_empty();
            object.ready |= events;
            if was_empty {
                mutable.ready_queue.push_back(id);
            }

            // Get a thread to unblock if any. Unblock only one thread at a time
            // to avoid thundering herd.
            mutable.waiters.pop_front()
        };

        if let Some(waiter) = waiter {
            waiter.unblock();
        }
    }
}

impl Handleable for Poll {}

pub fn sys_poll_create(current: &SharedRef<Thread>) -> Result<SyscallResult, ErrorCode> {
    let poll = Poll::new()?;
    let handle = Handle::new(poll, HandleRight::READ | HandleRight::WRITE);
    let handle_id = current.process().handles().lock().insert(handle)?;

    Ok(SyscallResult::Return(handle_id.as_usize()))
}

pub fn sys_poll_add(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
) -> Result<SyscallResult, ErrorCode> {
    let poll_id = HandleId::from_usize(a0)?;
    let listenee_id = HandleId::from_usize(a1)?;
    let interests = Readiness::from_raw(a2 as u8)?;

    let handle_table = current.process().handles().lock();
    let poll = handle_table
        .get::<Poll>(poll_id)?
        .authorize(HandleRight::WRITE)?;

    let listenee_handle = handle_table.get_any(listenee_id)?;
    poll.add(listenee_id, listenee_handle.clone(), interests)?;

    Ok(SyscallResult::Return(0))
}
pub fn sys_poll_update(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
) -> Result<SyscallResult, ErrorCode> {
    let poll_id = HandleId::from_usize(a0)?;
    let listenee_id = HandleId::from_usize(a1)?;
    let or_mask = Readiness::from_raw(a2 as u8)?;
    let and_mask = Readiness::from_raw(a3 as u8)?;

    let poll = current
        .process()
        .handles()
        .lock()
        .get::<Poll>(poll_id)?
        .authorize(HandleRight::WRITE)?;

    poll.update(listenee_id, or_mask, and_mask)?;

    Ok(SyscallResult::Return(0))
}

pub fn sys_poll_wait(current: &SharedRef<Thread>, a0: usize) -> Result<SyscallResult, ErrorCode> {
    let poll_id = HandleId::from_usize(a0)?;
    let poll_handle = current.process().handles().lock().get::<Poll>(poll_id)?;
    let poll = poll_handle.authorize(HandleRight::READ)?;
    poll.wait(current)
}

pub fn sys_poll_remove(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
) -> Result<SyscallResult, ErrorCode> {
    let poll_id = HandleId::from_usize(a0)?;
    let listenee_id = HandleId::from_usize(a1)?;

    let poll = current
        .process()
        .handles()
        .lock()
        .get::<Poll>(poll_id)?
        .authorize(HandleRight::WRITE)?;

    poll.remove(listenee_id)?;
    Ok(SyscallResult::Return(0))
}
