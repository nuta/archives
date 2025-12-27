#![no_std]

extern crate alloc;

pub use start::start;

#[macro_use]
pub mod log;

// Rust standard library modules.
pub mod collections;
pub use alloc::borrow;
pub use alloc::rc;

// FTL APIs.
pub mod channel;
pub mod error;
pub mod eventloop;
pub mod handle;
pub mod interrupt;
pub mod poll;
pub mod prelude;
pub mod process;
pub mod spec;
pub mod thread;
pub mod timer;
pub mod vmarea;
pub mod vmspace;

mod arch;
mod start;
mod syscall;
