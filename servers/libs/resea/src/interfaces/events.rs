#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 0;
pub const EVENTS_SERVICE: u16 = 0;
pub const EVENTS_NOTIFICATION_MSG: u16 = (EVENTS_SERVICE  << 8) | 1u16;
pub const EVENTS_NOTIFICATION_HEADER: u64 = ((EVENTS_NOTIFICATION_MSG as u64) << 16) | (0u64 << 8);


pub struct Events {
    cid: CId
}

impl Events {
    pub fn from_cid(cid: CId) -> Events {
        Events {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Events {
        Events {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Events {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(EVENTS_SERVICE).unwrap();
        Events {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn notification(&self, notification: usize) -> ServerResult<()> {
        unsafe {
            let __header = ipc_send(self.cid, EVENTS_NOTIFICATION_HEADER as Payload, notification as Payload, 0, 0, 0);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn notification(&self, from: Channel, notification: usize);


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            EVENTS_NOTIFICATION_MSG => {
                self.notification(from, a0 as usize);
                ((ErrorCode::DontReply as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}