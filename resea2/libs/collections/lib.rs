#![cfg_attr(not(test), no_std)]
#![feature(const_fn_trait_bound)]

pub mod linked_list;
mod static_trait;

pub use static_trait::Static;
