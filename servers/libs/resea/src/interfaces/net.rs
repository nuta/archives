#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 7;
pub const NET_SERVICE: u16 = 7;
pub const NET_CLOSE_MSG: u16 = (NET_SERVICE  << 8) | 1u16;
pub const NET_CLOSE_HEADER: u64 = ((NET_CLOSE_MSG as u64) << 16) | (0u64 << 8);
pub const NET_CLOSE_REPLY_MSG: u16 = (NET_SERVICE  << 8) | 1u16 + 1;
pub const NET_CLOSE_REPLY_HEADER: u64 = ((NET_CLOSE_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const NET_OPEN_UDP_MSG: u16 = (NET_SERVICE  << 8) | 7u16;
pub const NET_OPEN_UDP_HEADER: u64 = ((NET_OPEN_UDP_MSG as u64) << 16) | (1u64 << 8);
pub const NET_OPEN_UDP_REPLY_MSG: u16 = (NET_SERVICE  << 8) | 7u16 + 1;
pub const NET_OPEN_UDP_REPLY_HEADER: u64 = ((NET_OPEN_UDP_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct Net {
    cid: CId
}

impl Net {
    pub fn from_cid(cid: CId) -> Net {
        Net {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Net {
        Net {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Net {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(NET_SERVICE).unwrap();
        Net {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn close(&self, sock: usize) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, NET_CLOSE_HEADER as Payload, sock as Payload, 0, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn open_udp(&self, addr: &[u8], flags: u32) -> ServerResult<(usize)> {
        let mut __r: Payload = 0;
        let mut sock: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, NET_OPEN_UDP_HEADER as Payload, addr.as_ptr() as Payload, addr.len() as Payload, flags as Payload, 0, &mut sock, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((sock as usize))
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn close(&self, from: Channel, sock: usize) -> ServerResult<()>;


    fn open_udp(&self, from: Channel, addr: OoL, flags: u32) -> ServerResult<(usize)>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            NET_CLOSE_MSG => {
                match self.close(from, a0 as usize) {
                    Ok(()) => (NET_CLOSE_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (NET_CLOSE_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            NET_OPEN_UDP_MSG => {
                match self.open_udp(from, OoL::from_payload(a0 as usize, a1 as usize), a2 as u32) {
                    Ok((sock)) => (NET_OPEN_UDP_REPLY_HEADER | OK_HEADER, sock as Payload, 0, 0, 0),
                    Err(err) => (NET_OPEN_UDP_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}