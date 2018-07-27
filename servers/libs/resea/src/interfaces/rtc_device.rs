#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 102;
pub const RTC_DEVICE_SERVICE: u16 = 102;
pub const RTC_DEVICE_READ_MSG: u16 = (RTC_DEVICE_SERVICE  << 8) | 1u16;
pub const RTC_DEVICE_READ_HEADER: u64 = ((RTC_DEVICE_READ_MSG as u64) << 16) | (0u64 << 8);
pub const RTC_DEVICE_READ_REPLY_MSG: u16 = (RTC_DEVICE_SERVICE  << 8) | 1u16 + 1;
pub const RTC_DEVICE_READ_REPLY_HEADER: u64 = ((RTC_DEVICE_READ_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct RtcDevice {
    cid: CId
}

impl RtcDevice {
    pub fn from_cid(cid: CId) -> RtcDevice {
        RtcDevice {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> RtcDevice {
        RtcDevice {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> RtcDevice {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(RTC_DEVICE_SERVICE).unwrap();
        RtcDevice {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn read(&self) -> ServerResult<(u32, u32, u32)> {
        let mut __r: Payload = 0;
        let mut ymd: Payload = 0;
        
        let mut hms: Payload = 0;
        
        let mut nsec: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, RTC_DEVICE_READ_HEADER as Payload, 0, 0, 0, 0, &mut ymd, &mut hms, &mut nsec, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((ymd as u32, hms as u32, nsec as u32))
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn read(&self, from: Channel) -> ServerResult<(u32, u32, u32)>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            RTC_DEVICE_READ_MSG => {
                match self.read(from) {
                    Ok((ymd, hms, nsec)) => (RTC_DEVICE_READ_REPLY_HEADER | OK_HEADER, ymd as Payload, hms as Payload, nsec as Payload, 0),
                    Err(err) => (RTC_DEVICE_READ_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}