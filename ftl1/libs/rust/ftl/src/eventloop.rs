use alloc::rc::Rc;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::poll::Readiness;
use hashbrown::HashMap;

use crate::channel::Channel;
use crate::handle::Handleable;
use crate::interrupt::Interrupt;
use crate::poll::Poll;
use crate::thread::Thread;
use crate::timer::Timer;

#[derive(Debug)]
pub enum Error {
    PollCreate(ErrorCode),
    PollAdd(ErrorCode),
    PollUpdate(ErrorCode),
    PollWait(ErrorCode),
    PollRemove(ErrorCode),
    InvalidListenee(HandleId),
    UnexpectedReadiness(Readiness),
}

enum Object {
    Channel { ch: Rc<Channel> },
    Timer { timer: Rc<Timer> },
    Interrupt { interrupt: Rc<Interrupt> },
    Thread { thread: Rc<Thread> },
}

struct Item<C> {
    ctx: C,
    object: Object,
}

#[derive(Debug)]
pub enum Event<'a> {
    ChannelWritable { ch: &'a Rc<Channel> },
    ChannelReadable { ch: &'a Rc<Channel> },
    ChannelClosed { ch: &'a Rc<Channel> },
    TimerFired { timer: &'a Rc<Timer> },
    InterruptFired { interrupt: &'a Rc<Interrupt> },
    ThreadExited { thread: &'a Rc<Thread> },
}

pub struct EventLoop<C> {
    poll: Poll,
    items: HashMap<HandleId, Item<C>>,
}

impl<C> EventLoop<C> {
    /// Creates a new event loop.
    pub fn new() -> Result<Self, Error> {
        let poll = Poll::new().map_err(Error::PollCreate)?;
        Ok(Self {
            poll,
            items: HashMap::new(),
        })
    }

    /// Adds a channel object.
    pub fn add_channel<T>(&mut self, ch: T, ctx: C) -> Result<(), Error>
    where
        T: Into<Rc<Channel>>,
    {
        let ch: Rc<Channel> = ch.into();
        let id = ch.handle_id();
        let interests = Readiness::READABLE | Readiness::PEER_CLOSED;
        self.poll.add(id, interests).map_err(Error::PollAdd)?;

        let object = Object::Channel { ch };
        self.items.insert(id, Item { ctx, object });
        Ok(())
    }

    /// Adds a timer object.
    pub fn add_timer<T>(&mut self, timer: T, ctx: C) -> Result<(), Error>
    where
        T: Into<Rc<Timer>>,
    {
        let timer: Rc<Timer> = timer.into();
        let id = timer.handle_id();
        let interests = Readiness::READABLE;
        self.poll.add(id, interests).map_err(Error::PollAdd)?;

        let object = Object::Timer { timer };
        self.items.insert(id, Item { ctx, object });
        Ok(())
    }

    /// Adds an interrupt object.
    pub fn add_interrupt<T>(&mut self, interrupt: T, ctx: C) -> Result<(), Error>
    where
        T: Into<Rc<Interrupt>>,
    {
        let interrupt: Rc<Interrupt> = interrupt.into();
        let id = interrupt.handle_id();
        self.poll
            .add(id, Readiness::READABLE)
            .map_err(Error::PollAdd)?;

        let object = Object::Interrupt { interrupt };
        self.items.insert(id, Item { ctx, object });
        Ok(())
    }

    /// Adds a thread object.
    pub fn add_thread<T>(&mut self, thread: T, ctx: C) -> Result<(), Error>
    where
        T: Into<Rc<Thread>>,
    {
        let thread: Rc<Thread> = thread.into();
        let id = thread.handle_id();
        let interests = Readiness::PEER_CLOSED;
        self.poll.add(id, interests).map_err(Error::PollAdd)?;

        let object = Object::Thread { thread };
        self.items.insert(id, Item { ctx, object });
        Ok(())
    }

    /// Removes an object.
    pub fn remove(&mut self, id: HandleId) -> Result<(), Error> {
        self.poll.remove(id).map_err(Error::PollRemove)?;
        self.items.remove(&id);
        Ok(())
    }

    // TODO: Remove me
    pub fn get_context(&self, id: HandleId) -> Option<&C> {
        self.items.get(&id).map(|item| &item.ctx)
    }

    /// Enables the given interest bits (or-mask).
    pub fn listen(&mut self, id: HandleId, interests: Readiness) -> Result<(), Error> {
        self.poll
            .update(id, interests, Readiness::ALL)
            .map_err(Error::PollUpdate)?;
        Ok(())
    }

    /// Disables the given interest bits.
    pub fn unlisten(&mut self, id: HandleId, interests: Readiness) -> Result<(), Error> {
        self.poll
            .update(id, Readiness::NONE, !interests)
            .map_err(Error::PollUpdate)?;
        Ok(())
    }

    /// Waits for a next event.
    pub fn wait(&mut self) -> Result<(&mut C, Event<'_>), Error> {
        let (id, readiness) = self.poll.wait().map_err(Error::PollWait)?;
        let item = self
            .items
            .get_mut(&id)
            .ok_or(Error::InvalidListenee(id))?;

        match &item.object {
            Object::Channel { ch } => {
                if readiness.contains(Readiness::WRITABLE) {
                    Ok((&mut item.ctx, Event::ChannelWritable { ch }))
                } else if readiness.contains(Readiness::READABLE) {
                    Ok((&mut item.ctx, Event::ChannelReadable { ch }))
                } else if readiness == Readiness::PEER_CLOSED {
                    Ok((&mut item.ctx, Event::ChannelClosed { ch }))
                } else {
                    Err(Error::UnexpectedReadiness(readiness))
                }
            }
            Object::Timer { timer } => {
                if readiness.contains(Readiness::READABLE) {
                    Ok((&mut item.ctx, Event::TimerFired { timer }))
                } else {
                    Err(Error::UnexpectedReadiness(readiness))
                }
            }
            Object::Interrupt { interrupt } => {
                if readiness.contains(Readiness::READABLE) {
                    Ok((&mut item.ctx, Event::InterruptFired { interrupt }))
                } else {
                    Err(Error::UnexpectedReadiness(readiness))
                }
            }
            Object::Thread { thread } => {
                if readiness == Readiness::PEER_CLOSED {
                    Ok((&mut item.ctx, Event::ThreadExited { thread }))
                } else {
                    Err(Error::UnexpectedReadiness(readiness))
                }
            }
        }
    }
}
