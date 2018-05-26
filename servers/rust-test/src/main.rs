#![no_std]
#![feature(start)]

extern crate resea;
use resea::string::{String};

#[start]
fn main(_argc: isize, _argv: *const *const u8) -> isize {
    let mut string = String::new();
    string.push('c');
    string.push('A');
    0
}
