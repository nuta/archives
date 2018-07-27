#![allow(dead_code)]
#![allow(unused_imports)]
#![allow(unused_parens)]

use core::slice;
use {Channel, Result as ServerResult, OoL, ErrorCode, CId, Payload, Header, HeaderTrait, ERROR_OFFSET};
use interfaces::discovery;
use syscalls::{ipc_open, ipc_call, ipc_send, ipc_recv, ipc_replyrecv};

pub const SERVICE_ID: u16 = 4;
pub const IO_SERVICE: u16 = 4;
pub const IO_IOALLOC_MSG: u16 = (IO_SERVICE  << 8) | 1u16;
pub const IO_IOALLOC_HEADER: u64 = ((IO_IOALLOC_MSG as u64) << 16) | (0u64 << 8);
pub const IO_IOALLOC_REPLY_MSG: u16 = (IO_SERVICE  << 8) | 1u16 + 1;
pub const IO_IOALLOC_REPLY_HEADER: u64 = ((IO_IOALLOC_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const IO_PMALLOC_MSG: u16 = (IO_SERVICE  << 8) | 3u16;
pub const IO_PMALLOC_HEADER: u64 = ((IO_PMALLOC_MSG as u64) << 16) | (0u64 << 8);
pub const IO_PMALLOC_REPLY_MSG: u16 = (IO_SERVICE  << 8) | 3u16 + 1;
pub const IO_PMALLOC_REPLY_HEADER: u64 = ((IO_PMALLOC_REPLY_MSG as u64) << 16) | (0u64 << 8);


pub const IO_LISTEN_FOR_IRQ_MSG: u16 = (IO_SERVICE  << 8) | 5u16;
pub const IO_LISTEN_FOR_IRQ_HEADER: u64 = ((IO_LISTEN_FOR_IRQ_MSG as u64) << 16) | (8u64 << 8);
pub const IO_LISTEN_FOR_IRQ_REPLY_MSG: u16 = (IO_SERVICE  << 8) | 5u16 + 1;
pub const IO_LISTEN_FOR_IRQ_REPLY_HEADER: u64 = ((IO_LISTEN_FOR_IRQ_REPLY_MSG as u64) << 16) | (0u64 << 8);



pub struct Io {
    cid: CId
}

impl Io {
    pub fn from_cid(cid: CId) -> Io {
        Io {
            cid: cid
        }
    }

    pub fn from_channel(ch: &Channel) -> Io {
        Io {
            cid: ch.to_cid()
        }
    }

    pub fn connect() -> Io {
        let discovery = discovery::Discovery::from_cid(1);
        let ch = discovery.discover(IO_SERVICE).unwrap();
        Io {
            cid: ch.to_cid()
        }
    }

    // Stubs
    pub fn ioalloc(&self, base: u32, length: usize) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, IO_IOALLOC_HEADER as Payload, base as Payload, length as Payload, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn pmalloc(&self, vaddr: usize, paddr: usize, length: usize) -> ServerResult<(usize, usize)> {
        let mut __r: Payload = 0;
        let mut vaddr_allocated: Payload = 0;
        
        let mut paddr_allocated: Payload = 0;
        

        unsafe {
            let __header: Header = ipc_call(self.cid, IO_PMALLOC_HEADER as Payload, vaddr as Payload, paddr as Payload, length as Payload, 0, &mut vaddr_allocated, &mut paddr_allocated, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok((vaddr_allocated as usize, paddr_allocated as usize))
            } else {
                Err(__header.error_type())
            }
        }
    }


    pub fn listen_for_irq(&self, irq: u32, listener: Channel) -> ServerResult<()> {
        let mut __r: Payload = 0;

        unsafe {
            let __header: Header = ipc_call(self.cid, IO_LISTEN_FOR_IRQ_HEADER as Payload, irq as Payload, listener.to_cid() as Payload, 0, 0, &mut __r, &mut __r, &mut __r, &mut __r);
            if __header.error_type() == ErrorCode::ErrorNone as u8 {
                Ok(())
            } else {
                Err(__header.error_type())
            }
        }
    }


}

pub trait Server {
    fn ioalloc(&self, from: Channel, base: u32, length: usize) -> ServerResult<()>;


    fn pmalloc(&self, from: Channel, vaddr: usize, paddr: usize, length: usize) -> ServerResult<(usize, usize)>;


    fn listen_for_irq(&self, from: Channel, irq: u32, listener: Channel) -> ServerResult<()>;


}

impl Server {
    pub fn handle(&self, from: Channel, header: Header, a0: Payload, a1: Payload, a2: Payload, a3: Payload)
        -> (Header, Payload, Payload, Payload, Payload) {

        const OK_HEADER: Header = (ErrorCode::ErrorNone as u64) << ERROR_OFFSET;
        match header.msg_type() {
            IO_IOALLOC_MSG => {
                match self.ioalloc(from, a0 as u32, a1 as usize) {
                    Ok(()) => (IO_IOALLOC_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (IO_IOALLOC_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            IO_PMALLOC_MSG => {
                match self.pmalloc(from, a0 as usize, a1 as usize, a2 as usize) {
                    Ok((vaddr_allocated, paddr_allocated)) => (IO_PMALLOC_REPLY_HEADER | OK_HEADER, vaddr_allocated as Payload, paddr_allocated as Payload, 0, 0),
                    Err(err) => (IO_PMALLOC_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            IO_LISTEN_FOR_IRQ_MSG => {
                match self.listen_for_irq(from, a0 as u32, Channel::from_cid(a1 as CId)) {
                    Ok(()) => (IO_LISTEN_FOR_IRQ_REPLY_HEADER | OK_HEADER, 0, 0, 0, 0),
                    Err(err) => (IO_LISTEN_FOR_IRQ_REPLY_HEADER | (err as u64) << ERROR_OFFSET, 0, 0, 0, 0),
                }
            },
            _ => {
                ((ErrorCode::UnknownMsg as u64) << ERROR_OFFSET, 0, 0, 0, 0)
            },
        }
    }
}