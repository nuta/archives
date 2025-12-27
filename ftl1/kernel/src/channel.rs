use alloc::collections::VecDeque;
use alloc::vec::Vec;

use arrayvec::ArrayVec;
use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::handle::HandleRight;
use ftl_types::message::MESSAGE_DATA_LEN_MAX;
use ftl_types::message::MESSAGE_NUM_HANDLES_MAX;
use ftl_types::message::MessageInfo;
use ftl_types::poll::Readiness;

use crate::handle::AnyHandle;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::isolation::Isolation;
use crate::isolation::IsolationPtr;
use crate::isolation::IsolationSlice;
use crate::isolation::IsolationSliceMut;
use crate::poll::PollEmitter;
use crate::process::HandleTable;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;
use crate::thread::Thread;

/// The maximum number of messages in a queue.
const QUEUE_LEN_MAX: usize = 32;

/// A message queue entry.
struct MessageEntry {
    msginfo: MessageInfo,
    data: Vec<u8>,
    handles: ArrayVec<AnyHandle, MESSAGE_NUM_HANDLES_MAX>,
}

struct Mutable {
    /// The peer channel. If it's `None`, the peer is not connected anymore
    /// and sending a message will fail.
    peer: Option<SharedRef<Channel>>,
    /// The received message queue.
    ///
    /// The queue length is limited to `QUEUE_LEN_MAX`. If the queue is full,
    /// the channel will return an error when sending a message (backpressure).
    queue: VecDeque<MessageEntry>,
    receivers: VecDeque<SharedRef<Thread>>,
}

impl Mutable {
    fn new() -> Self {
        Mutable {
            peer: None,
            queue: VecDeque::new(),
            receivers: VecDeque::new(),
        }
    }
}

pub struct Channel {
    mutable: SpinLock<Mutable>,
    emitter: PollEmitter,
}

impl Channel {
    pub fn new() -> Result<(SharedRef<Channel>, SharedRef<Channel>), ErrorCode> {
        // Create two channel objects.
        let ch0 = Self::do_new()?;
        let ch1 = Self::do_new()?;

        // Connect the two channels each other.
        ch0.mutable.lock().peer = Some(ch1.clone());
        ch1.mutable.lock().peer = Some(ch0.clone());
        Ok((ch0, ch1))
    }

    fn do_new() -> Result<SharedRef<Channel>, ErrorCode> {
        SharedRef::new(Channel {
            mutable: SpinLock::new(Mutable::new()),
            emitter: PollEmitter::new(Readiness::WRITABLE),
        })
    }

    /// [`Self::send`] but for system calls.
    pub fn send_from_user(
        &self,
        isolation: &dyn Isolation,
        handle_table: &mut HandleTable,
        msginfo: MessageInfo,
        msgbuffer: IsolationSlice,
        handles: IsolationSlice,
    ) -> Result<(), ErrorCode> {
        if msginfo.data_len() > MESSAGE_DATA_LEN_MAX {
            debug_warn!("too large message data: {}", msginfo.data_len());
            return Err(ErrorCode::TooLarge);
        }

        // Copy message data into the kernel memory. Do this before locking
        // the peer channel for better performance. This memory copy might
        // take a long time.
        let data = msgbuffer.read_to_vec(isolation, 0, msginfo.data_len())?;

        // Move handles.
        //
        // In this phase, since we don't know the receiver process, we don't
        // move to the desination process, but keep ownership of them (AnyHandle)
        // in the message entry.
        let num_handles = msginfo.num_handles();
        let mut moved_handles = ArrayVec::new();
        if num_handles > 0 {
            // Note: Don't release this lock until we've moved all handles
            //       to guarantee that the second loop never fails.

            // First loop: make sure moving handles won't fail and there are
            //             not too many ones.
            let mut handle_ids: ArrayVec<HandleId, MESSAGE_NUM_HANDLES_MAX> = ArrayVec::new();
            for i in 0..num_handles {
                let handle_id = handles.read(isolation, i * size_of::<HandleId>())?;

                // SAFETY: unwrap() won't panic because it should have enough
                //         capacity up to MESSAGE_HANDLES_MAX_COUNT.
                handle_ids.try_push(handle_id).unwrap();

                if !handle_table.is_movable(handle_id) {
                    return Err(ErrorCode::HandleNotMovable);
                }
            }

            // Second loop: Remove handles from the current process.
            for i in 0..num_handles {
                // Note: Don't read the handle from the buffer again - user
                //       might have changed it.
                let handle_id = handle_ids[i];

                // SAFETY: unwrap() won't panic because we've checked the handle
                //         is movable in the previous loop.
                let handle = handle_table.take(handle_id).unwrap();

                // SAFETY: unwrap() won't panic because `handles` should have
                //         enough capacity up to MESSAGE_NUM_HANDLES_MAX.
                moved_handles.try_push(handle).unwrap();
            }
        }

        self.send(msginfo, data, moved_handles)?;
        Ok(())
    }

    /// Sends a message to the peer channel (non-blocking).
    ///
    /// If the queue is full, returns [`ErrorCode::Backpressure`] immediately.
    pub fn send(
        &self,
        msginfo: MessageInfo,
        data: Vec<u8>,
        handles: ArrayVec<AnyHandle, MESSAGE_NUM_HANDLES_MAX>,
    ) -> Result<(), ErrorCode> {
        debug_assert_eq!(data.len(), msginfo.data_len());
        debug_assert_eq!(msginfo.num_handles(), handles.len());

        let peer_ch = {
            // Clone the peer reference to avoid lock ordering issue.
            let mutable = self.mutable.lock();
            mutable.peer.as_ref().ok_or(ErrorCode::NoPeer)?.clone()
        };

        let mut peer_mutable = peer_ch.mutable.lock();

        // Allocate space for the message in the peer's queue so that
        // `VecDeque::push_back` won't panic.
        if peer_mutable.queue.try_reserve_exact(1).is_err() {
            return Err(ErrorCode::OutOfMemory);
        }

        // Check if the queue is full.
        if peer_mutable.queue.len() >= QUEUE_LEN_MAX {
            return Err(ErrorCode::Backpressure);
        }

        // The message is ready to be sent. Enqueue it.
        peer_mutable.queue.push_back(MessageEntry {
            msginfo,
            data,
            handles,
        });

        if let Some(receiver) = peer_mutable.receivers.pop_front() {
            receiver.unblock();
        }

        // The peer has at least one message to read now.
        peer_ch.emitter.set(Readiness::READABLE);

        if peer_mutable.queue.len() == QUEUE_LEN_MAX {
            // The queue is full. Mark the peer as not writable.
            peer_ch.emitter.clear(Readiness::WRITABLE);
        }

        Ok(())
    }

    /// Receives a message from the peer channel (non-blocking).
    ///
    /// If the queue is empty, returns [`ErrorCode::Empty`] immediately.
    pub fn try_recv(
        self: &SharedRef<Channel>,
        isolation: &dyn Isolation,
        handle_table: &mut HandleTable,
        msgbuffer: &mut IsolationSliceMut,
        handles: &mut IsolationSliceMut,
    ) -> Result<MessageInfo, ErrorCode> {
        let mut entry = {
            let mut mutable = self.mutable.lock();
            let entry = match mutable.queue.pop_front() {
                Some(entry) => entry,
                None => {
                    // Check if the peer is still connected. This is to allow
                    // the peer to close the channel without waiting for us to
                    // receive all messages.
                    return if mutable.peer.is_some() {
                        // We have no message to read *for now*. The peer might
                        // send a message later.
                        Err(ErrorCode::Empty)
                    } else {
                        // We'll never receive a message anymore. Tell the caller
                        // that you're done.
                        Err(ErrorCode::NoPeer)
                    };
                }
            };

            if mutable.queue.is_empty() {
                self.emitter.clear(Readiness::READABLE);
            }

            entry
        };

        // Install handles into the current (receiver) process.
        for (i, any_handle) in entry.handles.drain(..).enumerate() {
            // TODO: Define the expected behavior when it fails to add a handle.
            let handle_id = handle_table.insert(any_handle)?;
            handles.write(isolation, i * size_of::<HandleId>(), handle_id)?;
        }

        // Copy message data into the buffer.
        msgbuffer.write_bytes(isolation, 0, &entry.data[0..entry.msginfo.data_len()])?;

        // The queue is not full anymore since we've just popped
        // a message. Mark the channel as writable.
        self.emitter.set(Readiness::WRITABLE);

        Ok(entry.msginfo)
    }
}

impl Handleable for Channel {
    fn is_movable(&self) -> bool {
        true
    }

    fn close(&self) {
        // Remove the reference to us from the peer channel.
        let peer_ch = self.mutable.lock().peer.take();
        if let Some(peer) = peer_ch
            && peer.mutable.lock().peer.take().is_some()
        {
            // Notify the peer that we're gone.
            peer.emitter.set(Readiness::PEER_CLOSED);
        }
    }

    fn emitter(&self) -> Option<&PollEmitter> {
        Some(&self.emitter)
    }
}

pub fn sys_channel_create(current: &SharedRef<Thread>) -> Result<SyscallResult, ErrorCode> {
    let (ch0, ch1) = Channel::new()?;
    let handle0 = Handle::new(ch0, HandleRight::READ | HandleRight::WRITE);
    let handle1 = Handle::new(ch1, HandleRight::READ | HandleRight::WRITE);
    let id0 = current
        .process()
        .handles()
        .lock()
        .insert_two(handle0, handle1)?;
    Ok(SyscallResult::Return(id0.as_usize()))
}

pub fn sys_channel_send(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
) -> Result<SyscallResult, ErrorCode> {
    // Parse syscall arguments.
    let channel_id = HandleId::from_usize(a0)?;
    let msginfo = MessageInfo::from_raw(a1)?;
    let msgbuffer_ptr = IsolationPtr::new(a2);
    let handles_ptr = IsolationPtr::new(a3);
    let msgbuffer = IsolationSlice::new(msgbuffer_ptr, msginfo.data_len());
    let handles = IsolationSlice::new(handles_ptr, size_of::<HandleId>() * msginfo.num_handles());

    // Get the channel handle.
    let process = current.process();
    let mut handle_table = process.handles().lock();
    let ch = handle_table
        .get::<Channel>(channel_id)?
        .authorize(HandleRight::WRITE)?;

    ch.send_from_user(
        process.isolation(),
        &mut handle_table,
        msginfo,
        msgbuffer,
        handles,
    )?;

    Ok(SyscallResult::Return(0))
}

pub fn sys_channel_recv(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
) -> Result<SyscallResult, ErrorCode> {
    // Parse syscall arguments.
    let channel_id = HandleId::from_usize(a0)?;
    let msgbuffer_ptr = IsolationPtr::new(a1);
    let handle_ptr = IsolationPtr::new(a2);
    let mut msgbuffer = IsolationSliceMut::new(msgbuffer_ptr, MESSAGE_DATA_LEN_MAX);
    let mut handles =
        IsolationSliceMut::new(handle_ptr, size_of::<HandleId>() * MESSAGE_NUM_HANDLES_MAX);

    // Get the channel handle.
    let process = current.process();
    let mut handle_table = process.handles().lock();
    let ch = handle_table
        .get::<Channel>(channel_id)?
        .authorize(HandleRight::READ)?;

    let msginfo = ch.try_recv(
        process.isolation(),
        &mut handle_table,
        &mut msgbuffer,
        &mut handles,
    )?;

    Ok(SyscallResult::Return(msginfo.as_raw()))
}
