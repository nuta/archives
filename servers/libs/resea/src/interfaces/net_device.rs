#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 104;
pub const NET_DEVICE_SERVICE: u16 = 104;
pub const NET_DEVICE_SEND_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 1u16;
pub const NET_DEVICE_SEND_HEADER: u64 = ((NET_DEVICE_SEND_MSG as u64) << 16) | (1u64 << 8);
pub const NET_DEVICE_SEND_REPLY_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 1u16 + 1;
pub const NET_DEVICE_SEND_REPLY_HEADER: u64 = ((NET_DEVICE_SEND_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const NET_DEVICE_LISTEN_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 3u16;
pub const NET_DEVICE_LISTEN_HEADER: u64 = ((NET_DEVICE_LISTEN_MSG as u64) << 16) | (2u64 << 8);
pub const NET_DEVICE_LISTEN_REPLY_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 3u16 + 1;
pub const NET_DEVICE_LISTEN_REPLY_HEADER: u64 = ((NET_DEVICE_LISTEN_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const NET_DEVICE_GET_HWADDR_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 5u16;
pub const NET_DEVICE_GET_HWADDR_HEADER: u64 = ((NET_DEVICE_GET_HWADDR_MSG as u64) << 16) | (0u64 << 8);
pub const NET_DEVICE_GET_HWADDR_REPLY_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 5u16 + 1;
pub const NET_DEVICE_GET_HWADDR_REPLY_HEADER: u64 = ((NET_DEVICE_GET_HWADDR_REPLY_MSG as u64) << 16) | (1u64 << 8);


pub const NET_DEVICE_RECEIVED_MSG: u16 = (NET_DEVICE_SERVICE  << 8) | 7u16;
pub const NET_DEVICE_RECEIVED_HEADER: u64 = ((NET_DEVICE_RECEIVED_MSG as u64) << 16) | (1u64 << 8);


pub struct NetDevice {
    cid: CId
}

impl NetDevice {
    pub fn from_cid(cid: CId) -> NetDevice {
        NetDevice {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> NetDevice {
        NetDevice {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> NetDevice {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(NET_DEVICE_SERVICE).unwrap();
        NetDevice {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn send(&self, data: &[u8]) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, NET_DEVICE_SEND_HEADER as Payload, data.as_ptr() as Payload, data.len() as Payload, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn listen(&self, listener: Channel) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, NET_DEVICE_LISTEN_HEADER as Payload, listener.to_cid() as Payload, 0, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn get_hwaddr(&self) -> ServerResult<(OoL)> {
        let mut __r: Payload = 0;
        let mut hwaddr: Payload = 0;
        
        let mut hwaddr_length: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, NET_DEVICE_GET_HWADDR_HEADER as Payload, 0, 0, 0, 0, &mut hwaddr, &mut hwaddr_length, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((OoL::from_payload(hwaddr as usize, hwaddr_length as usize)))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn received(&self, data: &[u8]) -> ServerResult<()> {
        unsafe {
            let __header = ipc_send(self.cid, NET_DEVICE_RECEIVED_HEADER as Payload, data.as_ptr() as Payload, data.len() as Payload, 0, 0);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn send(&self, from: Channel, data: OoL) -> ServerResult<()>;


    fn listen(&self, from: Channel, listener: Channel) -> ServerResult<()>;


    fn get_hwaddr(&self, from: Channel) -> ServerResult<(&[u8])>;


    fn received(&self, from: Channel, data: OoL);


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            NET_DEVICE_SEND_MSG => {
                match self.send(from, OoL::from_payload(a0 as usize, a1 as usize)) {
                    Ok(()) => (NET_DEVICE_SEND_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (NET_DEVICE_SEND_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            NET_DEVICE_LISTEN_MSG => {
                match self.listen(from, Channel::from_cid(a0 as CId)) {
                    Ok(()) => (NET_DEVICE_LISTEN_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (NET_DEVICE_LISTEN_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            NET_DEVICE_GET_HWADDR_MSG => {
                match self.get_hwaddr(from) {
                    Ok((hwaddr)) => (NET_DEVICE_GET_HWADDR_REPLY_HEADER | OK_HEADER, hwaddr.as_ptr() as Payload, hwaddr.len() as Payload, 0, 0),
                    Err(err) => (NET_DEVICE_GET_HWADDR_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            NET_DEVICE_RECEIVED_MSG => {
                self.received(from, OoL::from_payload(a0 as usize, a1 as usize));
                ((ErrorCode::DontReply as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}