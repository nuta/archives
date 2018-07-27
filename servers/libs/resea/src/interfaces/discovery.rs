#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 3;
pub const DISCOVERY_SERVICE: u16 = 3;
pub const DISCOVERY_REGISTER_MSG: u16 = (DISCOVERY_SERVICE  << 8) | 1u16;
pub const DISCOVERY_REGISTER_HEADER: u64 = ((DISCOVERY_REGISTER_MSG as u64) << 16) | (8u64 << 8);
pub const DISCOVERY_REGISTER_REPLY_MSG: u16 = (DISCOVERY_SERVICE  << 8) | 1u16 + 1;
pub const DISCOVERY_REGISTER_REPLY_HEADER: u64 = ((DISCOVERY_REGISTER_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const DISCOVERY_DISCOVER_MSG: u16 = (DISCOVERY_SERVICE  << 8) | 3u16;
pub const DISCOVERY_DISCOVER_HEADER: u64 = ((DISCOVERY_DISCOVER_MSG as u64) << 16) | (0u64 << 8);
pub const DISCOVERY_DISCOVER_REPLY_MSG: u16 = (DISCOVERY_SERVICE  << 8) | 3u16 + 1;
pub const DISCOVERY_DISCOVER_REPLY_HEADER: u64 = ((DISCOVERY_DISCOVER_REPLY_MSG as u64) << 16) | (2u64 << 8);



pub struct Discovery {
    cid: CId
}

impl Discovery {
    pub fn from_cid(cid: CId) -> Discovery {
        Discovery {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Discovery {
        Discovery {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Discovery {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(DISCOVERY_SERVICE).unwrap();
        Discovery {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn register(&self, msg_type: u16, server: Channel) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, DISCOVERY_REGISTER_HEADER as Payload, msg_type as Payload, server.to_cid() as Payload, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn discover(&self, msg_type: u16) -> ServerResult<(Channel)> {
        let mut __r: Payload = 0;
        let mut client: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, DISCOVERY_DISCOVER_HEADER as Payload, msg_type as Payload, 0, 0, 0, &mut client, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((Channel::from_cid(client as CId)))
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn register(&self, from: Channel, msg_type: u16, server: Channel) -> ServerResult<()>;


    fn discover(&self, from: Channel, msg_type: u16) -> ServerResult<(Channel)>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            DISCOVERY_REGISTER_MSG => {
                match self.register(from, a0 as u16, Channel::from_cid(a1 as CId)) {
                    Ok(()) => (DISCOVERY_REGISTER_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (DISCOVERY_REGISTER_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            DISCOVERY_DISCOVER_MSG => {
                match self.discover(from, a0 as u16) {
                    Ok((client)) => (DISCOVERY_DISCOVER_REPLY_HEADER | OK_HEADER, client.to_cid() as Payload, 0, 0, 0),
                    Err(err) => (DISCOVERY_DISCOVER_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}