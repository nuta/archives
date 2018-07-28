#![feature(global_asm, alloc, pattern_parentheses)]
#![no_std]
#![allow(unused_variables)]
#![feature(asm)]

#[macro_use]
extern crate alloc;
use core::*;

pub mod arch;
pub use arch::prelude::*;
mod channel;
pub use channel::Channel;mod print;
mod server;
pub mod syscalls;
pub mod interfaces;

pub type Result<T> = core::result::Result<T, u8>;

#[derive(Debug)]
pub enum ErrorCode {
    ErrorNone = 0,
    UnknownMsg = 1,
    NotImplemented = 2,
    InvalidArg = 3,
    InvalidMsg = 4,
    NoMemory = 200,
    InvalidChannel = 201,
    ChannelNotLinked = 202,
    ChannelNotTransferred = 203,
    ChannelInUse = 204,
    ChannelClosed = 205,
    DontReply = 255,
}

impl Into<u8> for ErrorCode {
    fn into(self) -> u8 {
        self as u8
    }
}

pub struct OoL {
    addr: usize,
    len: usize,
}

impl<'a> OoL {
    pub fn from_payload(addr: usize, len: usize) -> OoL {
        OoL {
            addr: addr,
            len: len,
        }
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn as_slice(&'a self) -> &'a [u8] {
        unsafe { slice::from_raw_parts(self.addr as *const u8, self.len) }
    }
}

impl Drop for OoL {
    fn drop(&mut self) {
        // TODO: discard
    }
}
