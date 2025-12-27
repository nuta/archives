#![cfg_attr(not(test), no_std)]

mod dtb;
mod name;
mod node;
mod prop;
mod spec;

pub use dtb::Dtb;
pub use name::Name;
pub use node::Node;
pub use node::NodeIter;
pub use prop::Prop;
pub use prop::PropIter;

#[derive(Debug)]
pub enum Error {
    TooShort,
    InvalidMagic,
    ShorterThanTotalSize,
    NotAligned,
    UnknownToken(u32),
    NodeNameOutOfBounds,
    PropOutOfBounds,
    ValueSizeNotAligned,
    ValueSizeMismatch,
    MissingParent,
    NotNullTerminated,
}
