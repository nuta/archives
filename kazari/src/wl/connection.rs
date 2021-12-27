pub use crate::wl::wire::RawMessage;

#[derive(Debug, PartialEq)]
pub enum SendError {}

pub trait Connection {
    fn send(&self, m: RawMessage) -> Result<(), SendError>;
}
