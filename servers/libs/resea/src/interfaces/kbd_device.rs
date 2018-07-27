#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 101;
pub const KBD_DEVICE_SERVICE: u16 = 101;
pub const KBD_DEVICE_LISTEN_MSG: u16 = (KBD_DEVICE_SERVICE  << 8) | 1u16;
pub const KBD_DEVICE_LISTEN_HEADER: u64 = ((KBD_DEVICE_LISTEN_MSG as u64) << 16) | (2u64 << 8);
pub const KBD_DEVICE_LISTEN_REPLY_MSG: u16 = (KBD_DEVICE_SERVICE  << 8) | 1u16 + 1;
pub const KBD_DEVICE_LISTEN_REPLY_HEADER: u64 = ((KBD_DEVICE_LISTEN_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct KbdDevice {
    cid: CId
}

impl KbdDevice {
    pub fn from_cid(cid: CId) -> KbdDevice {
        KbdDevice {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> KbdDevice {
        KbdDevice {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> KbdDevice {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(KBD_DEVICE_SERVICE).unwrap();
        KbdDevice {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn listen(&self, listener: Channel) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, KBD_DEVICE_LISTEN_HEADER as Payload, listener.to_cid() as Payload, 0, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn listen(&self, from: Channel, listener: Channel) -> ServerResult<()>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            KBD_DEVICE_LISTEN_MSG => {
                match self.listen(from, Channel::from_cid(a0 as CId)) {
                    Ok(()) => (KBD_DEVICE_LISTEN_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (KBD_DEVICE_LISTEN_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}