use core::marker::PhantomData;
use core::slice;

global_asm!(include_str!("../../../libresea/arch/x64/syscall.S"));

#[cfg(target_arch="x86_64")]
pub mod x64;

pub const ERROR_OFFSET: u64 = 0;
pub const MINOR_ID_OFFSET: u64 = 16;
pub const MAJOR_ID_OFFSET: u64 = 24;

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
    DontReply = 255,
}

pub type CId = i64;
pub type Header = u64;
pub type Payload = u64;

pub trait HeaderTrait {
    fn msg_type(&self) -> u16;
    fn service_type(&self) -> u16;
    fn error_type(&self) -> u8;
}

impl HeaderTrait for Header {
    fn msg_type(&self) -> u16 {
        ((self >> MINOR_ID_OFFSET) & 0xffff) as u16
    }

    fn service_type(&self) -> u16 {
        ((self >> MAJOR_ID_OFFSET) & 0xff) as u16
    }

    fn error_type(&self) -> u8 {
        ((self >> ERROR_OFFSET) & 0xff) as u8
    }
}

pub struct OoL<'a, T> {
    slice: &'a [u8],
    pd: PhantomData<T>,
}

impl<'a, T> OoL<'a, T> {
    pub fn from_payload(addr: usize, len: usize) -> OoL<'a, T> {
        OoL {
            slice: unsafe { slice::from_raw_parts(addr as *const u8, len) },
            pd: PhantomData,
        }
    }

    pub fn len(&self) -> usize {
        self.slice.len()
    }

    pub fn as_slice(&self) -> &[u8] {
        self.slice
    }
}

impl<'a, T> Drop for OoL<'a, T> {
    fn drop(&mut self) {
        // TODO: discard
    }
}

extern "C" {
    pub fn ipc_open() -> CId;
    pub fn ipc_close(cid: CId);
    pub fn ipc_transfer(ch: CId, dest: CId);

    pub fn ipc_call(ch: CId, header: Header, a0: Payload, a1: Payload,
                    a2: Payload, a3: Payload, r0: *mut Payload, r1: *mut Payload,
                    r2: *mut Payload, r3: *mut Payload) -> Header;
    pub fn ipc_send(ch: CId, header: Header, a0: Payload, a1: Payload,
                    a2: Payload, a3: Payload) -> Header;
    pub fn ipc_recv(ch: CId, from: *mut CId, a0: *mut Payload, a1: *mut Payload,
                    a2: *mut Payload, a3: *mut Payload) -> Header;
    pub fn ipc_replyrecv(client: *mut CId, header: Header, r0: Payload, r1: Payload,
                         r2: Payload, r3: Payload, a0: *mut Payload, a1: *mut Payload,
                         a2: *mut Payload, a3: *mut Payload) -> Header;
}
