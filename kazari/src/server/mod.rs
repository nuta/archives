mod client;
mod keyboard;
mod layer;
mod objects;
mod pointer;
mod server;

pub use client::ClientId;
pub use keyboard::*;
pub use pointer::*;
pub use server::{Input, Server, ServerError};

const WINDOW_TITLE_BAR_HEIGHT: usize = 25;
