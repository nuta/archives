use core::mem::transmute;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum ErrorCode {
    __UnusedZero,
    OutOfMemory,
    OutOfSpace,
    HandleTypeMismatch,
    HandleNotFound,
    HandleNotMovable,
    NotAllowed,
    NotFound,
    SegmentationFault,
    AlreadyMapped,
    OutOfBounds,
    NotSupported,
    NoPeer,
    TooLarge,
    TooSmall,
    InvalidArgument,
    Empty,
    TooManyHandles,
    AlreadyExists,
    UnknownSyscall,
    TryLater,
    InvalidUri,
    Backpressure,
    InvalidMessage,
    // This must be the last entry for From<u8>.
    BadErrorCode,
}

impl From<u8> for ErrorCode {
    fn from(value: u8) -> Self {
        if value < ErrorCode::BadErrorCode as u8 {
            unsafe { transmute::<u8, ErrorCode>(value) }
        } else {
            ErrorCode::BadErrorCode
        }
    }
}
