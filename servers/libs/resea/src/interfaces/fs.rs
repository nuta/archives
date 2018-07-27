#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 6;
pub const FS_SERVICE: u16 = 6;
pub const FS_OPEN_MSG: u16 = (FS_SERVICE  << 8) | 1u16;
pub const FS_OPEN_HEADER: u64 = ((FS_OPEN_MSG as u64) << 16) | (1u64 << 8);
pub const FS_OPEN_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 1u16 + 1;
pub const FS_OPEN_REPLY_HEADER: u64 = ((FS_OPEN_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const FS_OPEN_DIR_MSG: u16 = (FS_SERVICE  << 8) | 3u16;
pub const FS_OPEN_DIR_HEADER: u64 = ((FS_OPEN_DIR_MSG as u64) << 16) | (1u64 << 8);
pub const FS_OPEN_DIR_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 3u16 + 1;
pub const FS_OPEN_DIR_REPLY_HEADER: u64 = ((FS_OPEN_DIR_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const FS_CLOSE_MSG: u16 = (FS_SERVICE  << 8) | 5u16;
pub const FS_CLOSE_HEADER: u64 = ((FS_CLOSE_MSG as u64) << 16) | (0u64 << 8);
pub const FS_CLOSE_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 5u16 + 1;
pub const FS_CLOSE_REPLY_HEADER: u64 = ((FS_CLOSE_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const FS_READ_MSG: u16 = (FS_SERVICE  << 8) | 7u16;
pub const FS_READ_HEADER: u64 = ((FS_READ_MSG as u64) << 16) | (0u64 << 8);
pub const FS_READ_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 7u16 + 1;
pub const FS_READ_REPLY_HEADER: u64 = ((FS_READ_REPLY_MSG as u64) << 16) | (1u64 << 8);


pub const FS_WRITE_MSG: u16 = (FS_SERVICE  << 8) | 9u16;
pub const FS_WRITE_HEADER: u64 = ((FS_WRITE_MSG as u64) << 16) | (16u64 << 8);
pub const FS_WRITE_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 9u16 + 1;
pub const FS_WRITE_REPLY_HEADER: u64 = ((FS_WRITE_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const FS_READDIR_MSG: u16 = (FS_SERVICE  << 8) | 11u16;
pub const FS_READDIR_HEADER: u64 = ((FS_READDIR_MSG as u64) << 16) | (0u64 << 8);
pub const FS_READDIR_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 11u16 + 1;
pub const FS_READDIR_REPLY_HEADER: u64 = ((FS_READDIR_REPLY_MSG as u64) << 16) | (1u64 << 8);


pub const FS_STAT_MSG: u16 = (FS_SERVICE  << 8) | 13u16;
pub const FS_STAT_HEADER: u64 = ((FS_STAT_MSG as u64) << 16) | (4u64 << 8);
pub const FS_STAT_REPLY_MSG: u16 = (FS_SERVICE  << 8) | 13u16 + 1;
pub const FS_STAT_REPLY_HEADER: u64 = ((FS_STAT_REPLY_MSG as u64) << 16) | (1u64 << 8);



pub struct Fs {
    cid: CId
}

impl Fs {
    pub fn from_cid(cid: CId) -> Fs {
        Fs {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Fs {
        Fs {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Fs {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(FS_SERVICE).unwrap();
        Fs {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn open(&self, path: &[u8]) -> ServerResult<(isize)> {
        let mut __r: Payload = 0;
        let mut fd: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_OPEN_HEADER as Payload, path.as_ptr() as Payload, path.len() as Payload, 0, 0, &mut fd, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((fd as isize))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn open_dir(&self, path: &[u8]) -> ServerResult<(isize)> {
        let mut __r: Payload = 0;
        let mut fd: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_OPEN_DIR_HEADER as Payload, path.as_ptr() as Payload, path.len() as Payload, 0, 0, &mut fd, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((fd as isize))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn close(&self, fd: isize) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_CLOSE_HEADER as Payload, fd as Payload, 0, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn read(&self, fd: isize, offset: u64, length: usize) -> ServerResult<(OoL)> {
        let mut __r: Payload = 0;
        let mut data: Payload = 0;
        
        let mut data_length: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_READ_HEADER as Payload, fd as Payload, offset as Payload, length as Payload, 0, &mut data, &mut data_length, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((OoL::from_payload(data as usize, data_length as usize)))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn write(&self, fd: isize, offset: u64, data: &[u8]) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_WRITE_HEADER as Payload, fd as Payload, offset as Payload, data.as_ptr() as Payload, data.len() as Payload, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn readdir(&self, fd: isize, offset: u64) -> ServerResult<(OoL)> {
        let mut __r: Payload = 0;
        let mut path: Payload = 0;
        
        let mut path_length: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_READDIR_HEADER as Payload, fd as Payload, offset as Payload, 0, 0, &mut path, &mut path_length, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((OoL::from_payload(path as usize, path_length as usize)))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn stat(&self, fd: isize, path: &[u8]) -> ServerResult<(OoL)> {
        let mut __r: Payload = 0;
        let mut stat: Payload = 0;
        
        let mut stat_length: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, FS_STAT_HEADER as Payload, fd as Payload, path.as_ptr() as Payload, path.len() as Payload, 0, &mut stat, &mut stat_length, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((OoL::from_payload(stat as usize, stat_length as usize)))
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn open(&self, from: Channel, path: OoL) -> ServerResult<(isize)>;


    fn open_dir(&self, from: Channel, path: OoL) -> ServerResult<(isize)>;


    fn close(&self, from: Channel, fd: isize) -> ServerResult<()>;


    fn read(&self, from: Channel, fd: isize, offset: u64, length: usize) -> ServerResult<(&[u8])>;


    fn write(&self, from: Channel, fd: isize, offset: u64, data: OoL) -> ServerResult<()>;


    fn readdir(&self, from: Channel, fd: isize, offset: u64) -> ServerResult<(&[u8])>;


    fn stat(&self, from: Channel, fd: isize, path: OoL) -> ServerResult<(&[u8])>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            FS_OPEN_MSG => {
                match self.open(from, OoL::from_payload(a0 as usize, a1 as usize)) {
                    Ok((fd)) => (FS_OPEN_REPLY_HEADER | OK_HEADER, fd as Payload, 0, 0, 0),
                    Err(err) => (FS_OPEN_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            FS_OPEN_DIR_MSG => {
                match self.open_dir(from, OoL::from_payload(a0 as usize, a1 as usize)) {
                    Ok((fd)) => (FS_OPEN_DIR_REPLY_HEADER | OK_HEADER, fd as Payload, 0, 0, 0),
                    Err(err) => (FS_OPEN_DIR_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            FS_CLOSE_MSG => {
                match self.close(from, a0 as isize) {
                    Ok(()) => (FS_CLOSE_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (FS_CLOSE_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            FS_READ_MSG => {
                match self.read(from, a0 as isize, a1 as u64, a2 as usize) {
                    Ok((data)) => (FS_READ_REPLY_HEADER | OK_HEADER, data.as_ptr() as Payload, data.len() as Payload, 0, 0),
                    Err(err) => (FS_READ_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            FS_WRITE_MSG => {
                match self.write(from, a0 as isize, a1 as u64, OoL::from_payload(a2 as usize, a3 as usize)) {
                    Ok(()) => (FS_WRITE_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (FS_WRITE_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            FS_READDIR_MSG => {
                match self.readdir(from, a0 as isize, a1 as u64) {
                    Ok((path)) => (FS_READDIR_REPLY_HEADER | OK_HEADER, path.as_ptr() as Payload, path.len() as Payload, 0, 0),
                    Err(err) => (FS_READDIR_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            FS_STAT_MSG => {
                match self.stat(from, a0 as isize, OoL::from_payload(a1 as usize, a2 as usize)) {
                    Ok((stat)) => (FS_STAT_REPLY_HEADER | OK_HEADER, stat.as_ptr() as Payload, stat.len() as Payload, 0, 0),
                    Err(err) => (FS_STAT_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}