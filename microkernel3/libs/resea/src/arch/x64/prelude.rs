pub const ERROR_OFFSET: u64 = 0;
pub const MINOR_ID_OFFSET: u64 = 16;
pub const MAJOR_ID_OFFSET: u64 = 24;

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
