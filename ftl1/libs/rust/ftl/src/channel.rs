use core::fmt;
use core::mem;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::message::CONNECT_MSG;
pub use ftl_types::message::Cookie;
use ftl_types::message::DATA_MSG;
use ftl_types::message::ERROR_REPLY_MSG;
pub use ftl_types::message::MESSAGE_DATA_LEN_MAX;
use ftl_types::message::MESSAGE_NUM_HANDLES_MAX;
use ftl_types::message::MessageInfo;
use ftl_types::message::OPEN_MSG;
use ftl_types::message::OPEN_REPLY_MSG;
use ftl_types::syscall::SYS_CHANNEL_CREATE;
use ftl_types::syscall::SYS_CHANNEL_RECV;
use ftl_types::syscall::SYS_CHANNEL_SEND;
use log::warn;

use crate::handle::Handleable;
use crate::handle::OwnedHandle;
use crate::syscall::syscall0;
use crate::syscall::syscall4;

pub enum Message<'a> {
    Data { data: &'a [u8] },
    Connect { ch: Channel, uri: &'a [u8] },
    Open { cookie: Cookie, uri: &'a [u8] },
    OpenReply { cookie: Cookie, ch: Channel },
    ErrorReply { cookie: Cookie, error: ErrorCode },
}

impl<'a> fmt::Debug for Message<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Message::Data { data } => {
                f.debug_struct("Data")
                    .field("data_len", &data.len())
                    .finish()
            }
            Message::Connect { ch, uri } => {
                f.debug_struct("Connect")
                    .field("ch", ch)
                    .field("uri", &uri)
                    .finish()
            }
            Message::Open { cookie, uri } => {
                f.debug_struct("Open")
                    .field("cookie", cookie)
                    .field("uri", &uri)
                    .finish()
            }
            Message::OpenReply { cookie, ch } => {
                f.debug_struct("OpenReply")
                    .field("cookie", cookie)
                    .field("ch", ch)
                    .finish()
            }
            Message::ErrorReply { cookie, error } => {
                f.debug_struct("ErrorReply")
                    .field("cookie", cookie)
                    .field("error", error)
                    .finish()
            }
        }
    }
}

pub struct MessageBuffer {
    data: [u8; MESSAGE_DATA_LEN_MAX],
    handles: [HandleId; MESSAGE_NUM_HANDLES_MAX],
}

impl MessageBuffer {
    pub const fn new() -> Self {
        Self {
            data: [0; MESSAGE_DATA_LEN_MAX],
            handles: [HandleId::from_raw(0); MESSAGE_NUM_HANDLES_MAX],
        }
    }
}

#[derive(Debug)]
pub struct Channel {
    handle: OwnedHandle,
}

impl Channel {
    pub fn new() -> Result<(Channel, Channel), ErrorCode> {
        let ret = syscall0(SYS_CHANNEL_CREATE)?;
        let handle0 = HandleId::from_raw(ret as i32);
        let handle1 = HandleId::from_raw(ret as i32 + 1);
        let ch0 = Channel::from_handle(OwnedHandle::from_raw(handle0));
        let ch1 = Channel::from_handle(OwnedHandle::from_raw(handle1));
        Ok((ch0, ch1))
    }

    pub fn from_handle(handle: OwnedHandle) -> Self {
        Self { handle }
    }

    pub fn send(&self, message: Message<'_>) -> Result<(), ErrorCode> {
        let (msginfo, data, handles): (MessageInfo, &[u8], &[HandleId]) = match message {
            Message::Data { data } => {
                let msginfo = MessageInfo::new(DATA_MSG, Cookie::new(0), data.len() as u16, 0);
                (msginfo, data, &[])
            }
            Message::Connect { ch, uri } => {
                let msginfo = MessageInfo::new(CONNECT_MSG, Cookie::new(0), uri.len() as u16, 1);

                let ch_id = ch.handle.id();
                mem::forget(ch);

                (msginfo, uri, &[ch_id])
            }
            Message::Open { cookie, uri } => {
                let msginfo = MessageInfo::new(OPEN_MSG, cookie, uri.len() as u16, 0);
                (msginfo, uri, &[])
            }
            Message::OpenReply { cookie, ch } => {
                let msginfo = MessageInfo::new(OPEN_REPLY_MSG, cookie, 0, 1);
                let handle_id = ch.handle.id();
                mem::forget(ch);
                (msginfo, &[], &[handle_id])
            }
            Message::ErrorReply { cookie, error } => {
                let error_byte = error as u8;
                let msginfo = MessageInfo::new(ERROR_REPLY_MSG, cookie, 0, 0);
                (msginfo, &[error_byte], &[])
            }
        };

        syscall4(
            SYS_CHANNEL_SEND,
            self.handle.id().as_usize(),
            msginfo.as_raw(),
            data.as_ptr() as usize,
            handles.as_ptr() as usize,
        )?;
        Ok(())
    }

    pub fn recv<'a>(&self, buffer: &'a mut MessageBuffer) -> Result<Message<'a>, ErrorCode> {
        let ret = syscall4(
            SYS_CHANNEL_RECV,
            self.handle.id().as_usize(),
            buffer.data.as_mut_ptr() as usize,
            buffer.handles.as_mut_ptr() as usize,
            0,
        )?;
        let msginfo = MessageInfo::from_raw(ret)?;
        let message = parse_message(msginfo, &buffer.data, &buffer.handles)
            .ok_or(ErrorCode::InvalidMessage)?;
        Ok(message)
    }

    pub fn handle(&self) -> &OwnedHandle {
        &self.handle
    }
}

fn parse_message<'a>(
    msginfo: MessageInfo,
    buf: &'a [u8],
    handles: &'a [HandleId],
) -> Option<Message<'a>> {
    match msginfo.kind() {
        DATA_MSG => {
            Some(Message::Data {
                data: &buf[..msginfo.data_len()],
            })
        }
        CONNECT_MSG => {
            debug_assert_eq!(msginfo.num_handles(), 1);
            let ch_id = handles[0];
            let ch_handle = OwnedHandle::from_raw(ch_id);
            Some(Message::Connect {
                ch: Channel::from_handle(ch_handle),
                uri: &buf[..msginfo.data_len()],
            })
        }
        OPEN_MSG => {
            Some(Message::Open {
                cookie: msginfo.cookie(),
                uri: &buf[..msginfo.data_len()],
            })
        }
        OPEN_REPLY_MSG => {
            if msginfo.num_handles() != 1 {
                warn!(
                    "invalid open reply message: num_handles = {}",
                    msginfo.num_handles()
                );
                return None;
            }

            Some(Message::OpenReply {
                cookie: msginfo.cookie(),
                ch: Channel::from_handle(OwnedHandle::from_raw(handles[0])),
            })
        }
        ERROR_REPLY_MSG => {
            if msginfo.data_len() != 1 {
                warn!(
                    "invalid error reply message: data_len = {}",
                    msginfo.data_len()
                );
                return None;
            }

            let cookie = msginfo.cookie();
            let error = ErrorCode::from(buf[0]);
            Some(Message::ErrorReply { cookie, error })
        }
        _ => {
            warn!("unknown message kind {}", msginfo.kind());
            None
        }
    }
}

impl Handleable for Channel {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}

impl<'de> serde::Deserialize<'de> for Channel {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let handle_id: i32 = serde::Deserialize::deserialize(deserializer)?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(handle_id));
        Ok(Channel { handle })
    }
}
