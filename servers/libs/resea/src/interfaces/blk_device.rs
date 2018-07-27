#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 100;
pub const BLK_DEVICE_SERVICE: u16 = 100;
pub const BLK_DEVICE_READ_MSG: u16 = (BLK_DEVICE_SERVICE  << 8) | 1u16;
pub const BLK_DEVICE_READ_HEADER: u64 = ((BLK_DEVICE_READ_MSG as u64) << 16) | (0u64 << 8);
pub const BLK_DEVICE_READ_REPLY_MSG: u16 = (BLK_DEVICE_SERVICE  << 8) | 1u16 + 1;
pub const BLK_DEVICE_READ_REPLY_HEADER: u64 = ((BLK_DEVICE_READ_REPLY_MSG as u64) << 16) | (1u64 << 8);


pub const BLK_DEVICE_WRITE_MSG: u16 = (BLK_DEVICE_SERVICE  << 8) | 3u16;
pub const BLK_DEVICE_WRITE_HEADER: u64 = ((BLK_DEVICE_WRITE_MSG as u64) << 16) | (4u64 << 8);
pub const BLK_DEVICE_WRITE_REPLY_MSG: u16 = (BLK_DEVICE_SERVICE  << 8) | 3u16 + 1;
pub const BLK_DEVICE_WRITE_REPLY_HEADER: u64 = ((BLK_DEVICE_WRITE_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct BlkDevice {
    cid: CId
}

impl BlkDevice {
    pub fn from_cid(cid: CId) -> BlkDevice {
        BlkDevice {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> BlkDevice {
        BlkDevice {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> BlkDevice {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(BLK_DEVICE_SERVICE).unwrap();
        BlkDevice {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn read(&self, offset: u64, length: usize) -> ServerResult<(OoL)> {
        let mut __r: Payload = 0;
        let mut data: Payload = 0;
        
        let mut data_length: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, BLK_DEVICE_READ_HEADER as Payload, offset as Payload, length as Payload, 0, 0, &mut data, &mut data_length, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((OoL::from_payload(data as usize, data_length as usize)))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn write(&self, offset: u64, data: &[u8]) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, BLK_DEVICE_WRITE_HEADER as Payload, offset as Payload, data.as_ptr() as Payload, data.len() as Payload, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn read(&self, from: Channel, offset: u64, length: usize) -> ServerResult<(&[u8])>;


    fn write(&self, from: Channel, offset: u64, data: OoL) -> ServerResult<()>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            BLK_DEVICE_READ_MSG => {
                match self.read(from, a0 as u64, a1 as usize) {
                    Ok((data)) => (BLK_DEVICE_READ_REPLY_HEADER | OK_HEADER, data.as_ptr() as Payload, data.len() as Payload, 0, 0),
                    Err(err) => (BLK_DEVICE_READ_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            BLK_DEVICE_WRITE_MSG => {
                match self.write(from, a0 as u64, OoL::from_payload(a1 as usize, a2 as usize)) {
                    Ok(()) => (BLK_DEVICE_WRITE_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (BLK_DEVICE_WRITE_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}