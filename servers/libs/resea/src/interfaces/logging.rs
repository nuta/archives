#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 2;
pub const LOGGING_SERVICE: u16 = 2;
pub const LOGGING_EMIT_MSG: u16 = (LOGGING_SERVICE  << 8) | 1u16;
pub const LOGGING_EMIT_HEADER: u64 = ((LOGGING_EMIT_MSG as u64) << 16) | (1u64 << 8);
pub const LOGGING_EMIT_REPLY_MSG: u16 = (LOGGING_SERVICE  << 8) | 1u16 + 1;
pub const LOGGING_EMIT_REPLY_HEADER: u64 = ((LOGGING_EMIT_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct Logging {
    cid: CId
}

impl Logging {
    pub fn from_cid(cid: CId) -> Logging {
        Logging {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Logging {
        Logging {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Logging {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(LOGGING_SERVICE).unwrap();
        Logging {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn emit(&self, str: &[u8]) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, LOGGING_EMIT_HEADER as Payload, str.as_ptr() as Payload, str.len() as Payload, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn emit(&self, from: Channel, str: OoL) -> ServerResult<()>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            LOGGING_EMIT_MSG => {
                match self.emit(from, OoL::from_payload(a0 as usize, a1 as usize)) {
                    Ok(()) => (LOGGING_EMIT_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (LOGGING_EMIT_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}