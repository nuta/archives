#![cfg_attr(target_os = "none", no_std)]
#![cfg_attr(target_os = "none", no_main)]
#![cfg_attr(not(target_os = "none"), allow(unused))]
// For SharedRef, our own Arc-like reference-counting.
#![feature(arbitrary_self_types)]
#![feature(coerce_unsized)]
#![feature(unsize)]
// For trap handlers that require alignment.
#![feature(fn_align)]

extern crate alloc;

#[macro_use]
mod print;

// Hardware abstraction layer.
mod arch;

// Rust utilities.
mod address;
mod global_allocator;
mod memory;
mod panic;
mod shared_ref;
mod spinlock;

// Kernel core components.
mod boot;
mod cpuvar;
mod device_tree;
mod handle;
mod scheduler;
mod startup;
mod timer;

// Kernel objects.
mod channel;
mod interrupt;
mod isolation;
mod poll;
mod process;
mod syscall;
mod thread;
mod vmarea;
mod vmspace;

/// This main function is unused. This is defined to make `cargo test` work.
#[cfg(not(target_os = "none"))]
pub fn main() {
    unreachable!()
}
