use crate::error::ErrorCode;

pub const MESSAGE_NUM_HANDLES_MAX: usize = 4;
pub const MESSAGE_DATA_LEN_MAX: usize = 4096;

/// The message metadata.
#[derive(Clone, Copy, Debug)]
#[repr(transparent)]
pub struct MessageInfo(i32);

const DATA_LEN_BITS: usize = 13;
const HANDLES_BITS: usize = 2;
const KIND_BITS: usize = 4;
const COOKIE_BITS: usize = 12;

const DATA_LEN_SHIFT: usize = 0;
const HANDLES_SHIFT: usize = DATA_LEN_SHIFT + DATA_LEN_BITS;
const KIND_SHIFT: usize = HANDLES_SHIFT + HANDLES_BITS;
const COOKIE_SHIFT: usize = KIND_SHIFT + KIND_BITS;

const DATA_LEN_MASK: i32 = (1 << DATA_LEN_BITS) - 1;
const HANDLES_MASK: i32 = (1 << HANDLES_BITS) - 1;
const KIND_MASK: i32 = (1 << KIND_BITS) - 1;
const COOKIE_MASK: i32 = (1 << COOKIE_BITS) - 1;

pub const DATA_MSG: u8 = 1;
pub const CONNECT_MSG: u8 = 2;
pub const OPEN_MSG: u8 = 3;
pub const OPEN_REPLY_MSG: u8 = 4;
pub const ERROR_REPLY_MSG: u8 = 5;

/// An opaque value that you may freely choose.
///
/// This is similar to HTTP cookies, except that clients send it to
/// the server, and the server sends it back to the client:
///
/// 1. Client sends an `Open` request to the server with a cookie A.
/// 2. Server receives the request and sends a reply with cookie A.
/// 3. Client receives the reply and uses the cookie A to identify the request.
#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub struct Cookie(u16);

impl Cookie {
    pub const fn new(raw: u16) -> Self {
        debug_assert!(raw < (1 << COOKIE_BITS));
        Self(raw)
    }

    pub const fn as_raw(self) -> u16 {
        self.0
    }
}

/// The message metadata.
///
/// - MSB: Always 0 to make it a positive number. The system call requires this
///   to distinguish it from error codes (negative numbers).
/// - `cookie`: The message-type specific data (12 bits).
/// - `kind`: The message type (4 bits).
/// - `num_handles`: The number of handles (2 bits).
/// - `data_len`: The length of the data (13 bits).
impl MessageInfo {
    pub const fn new(kind: u8, cookie: Cookie, data_len: u16, num_handles: u8) -> Self {
        debug_assert!(num_handles <= (1 << HANDLES_BITS));
        debug_assert!(data_len < (1 << DATA_LEN_BITS));
        debug_assert!(kind < (1 << KIND_BITS));

        let raw = (cookie.as_raw() as i32) << COOKIE_SHIFT
            | ((kind as i32) << KIND_SHIFT)
            | ((num_handles as i32) << HANDLES_SHIFT)
            | (data_len as i32);

        debug_assert!(raw >= 0);
        MessageInfo(raw)
    }

    pub fn as_raw(self) -> usize {
        self.0 as usize
    }

    pub fn from_raw(value: usize) -> Result<Self, ErrorCode> {
        debug_assert!(value <= i32::MAX as usize);
        Ok(MessageInfo(value as i32))
    }

    pub fn kind(self) -> u8 {
        ((self.0 >> KIND_SHIFT) & KIND_MASK) as u8
    }

    pub fn cookie(self) -> Cookie {
        Cookie::new(((self.0 >> COOKIE_SHIFT) & COOKIE_MASK) as u16)
    }

    pub fn data_len(self) -> usize {
        ((self.0 >> DATA_LEN_SHIFT) & DATA_LEN_MASK) as usize
    }

    pub fn num_handles(self) -> usize {
        ((self.0 >> HANDLES_SHIFT) & HANDLES_MASK) as usize
    }
}
