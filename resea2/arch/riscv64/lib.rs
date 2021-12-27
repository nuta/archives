#![no_std]
#![feature(asm)]
#![feature(global_asm)]
#![feature(const_fn_trait_bound)]
#![feature(naked_functions)]
#![allow(dead_code)]

#[macro_use]
#[allow(unused_imports)]
extern crate print;
#[macro_use]
extern crate cfg_if;

mod asm;
mod boot;
mod convert;
mod machines;
mod mmio;
mod registers;
mod thread;
mod trap;

#[cfg(feature = "riscv64_qemu")]
pub use crate::machines::qemu as machine;
#[cfg(feature = "hosting")]
pub use crate::machines::skelton as machine;

pub use convert::TryAsVAddr;
pub use machine::PAGE_SIZE;
pub use thread::{switch_thread, ArchThread};
