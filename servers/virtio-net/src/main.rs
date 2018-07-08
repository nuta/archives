#![no_std]

#[macro_use]
extern crate resea;
extern crate virtio;
mod device;

use resea::arch::{ErrorCode, OoL};
use resea::channel::{Channel};
use resea::server::{ServerResult};
use resea::interfaces::net_device;
use resea::interfaces::net_device::{Server as NetDeviceServer};
use device::{VirtioNet};

struct VirtioNetServer {
    device: VirtioNet,
    ch: Channel,
}

impl VirtioNetServer {
    pub fn new() -> VirtioNetServer {
        VirtioNetServer {
            device: VirtioNet::new(0xc040),
            ch: Channel::create(),
        }
    }
}

impl NetDeviceServer for VirtioNetServer {
    fn send(&self, from: Channel, data: OoL<&[u8]>) -> ServerResult<()> {
        /* TODO */
        Err(ErrorCode::NotImplemented)
    }
}

fn main() {
    println!("virtio: starting virtio driver...");
    let server = VirtioNetServer::new();
    register_as!(&server, [net_device]);
    serve_forever!(&server, [net_device]);
}
