#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 1;
pub const EXIT_SERVICE: u16 = 1;
pub const EXIT_EXIT_MSG: u16 = (EXIT_SERVICE  << 8) | 1u16;
pub const EXIT_EXIT_HEADER: u64 = ((EXIT_EXIT_MSG as u64) << 16) | (0u64 << 8);
pub const EXIT_EXIT_REPLY_MSG: u16 = (EXIT_SERVICE  << 8) | 1u16 + 1;
pub const EXIT_EXIT_REPLY_HEADER: u64 = ((EXIT_EXIT_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct Exit {
    cid: CId
}

impl Exit {
    pub fn from_cid(cid: CId) -> Exit {
        Exit {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Exit {
        Exit {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Exit {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(EXIT_SERVICE).unwrap();
        Exit {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn exit(&self, exit_code: u8) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, EXIT_EXIT_HEADER as Payload, exit_code as Payload, 0, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn exit(&self, from: Channel, exit_code: u8) -> ServerResult<()>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            EXIT_EXIT_MSG => {
                match self.exit(from, a0 as u8) {
                    Ok(()) => (EXIT_EXIT_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (EXIT_EXIT_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}