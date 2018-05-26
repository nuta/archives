#![no_std]
#![feature(start)]

extern crate resea;
use resea::interfaces::logging::{Logging};
use resea::string::{String};

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let logging = Logging::from_cid(1);
    logging.emit(b"Hello");

    let mut string = String::new();
    string.push('c');
    string.push('A');
    0
}
