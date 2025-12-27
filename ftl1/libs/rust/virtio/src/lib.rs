#![no_std]

pub mod free_list;
pub mod transports;
pub mod virtqueue;

#[derive(Debug, PartialEq, Eq)]
pub enum DeviceType {
    Unknown = 0,
    Net = 1,
    Blk = 2,
}
