#![no_std]

#[macro_use]
extern crate resea;
extern crate virtio;
mod device;

use core::slice;
use resea::arch::{ErrorCode, OoL};
use resea::channel::{Channel};
use resea::server::{ServerResult};
use resea::interfaces::blk_device;
use resea::interfaces::blk_device::{Server as BlkDeviceServer};
use device::{VirtioBlk, SECTOR_SIZE};

struct VirtioBlkServer {
    device: VirtioBlk,
    ch: Channel,
}

impl VirtioBlkServer {
    pub fn new() -> VirtioBlkServer {
        VirtioBlkServer {
            device: VirtioBlk::new(0xc040),
            ch: Channel::create(),
        }
    }
}

impl BlkDeviceServer for VirtioBlkServer {
    fn read(&self, from: Channel, offset: u64, length: usize) -> ServerResult<(&[u8])> {
        let sector = offset / SECTOR_SIZE as u64;
        let ptr = self.device.read(sector, length);
        let data = unsafe { slice::from_raw_parts(ptr, length) };
        Ok((data))
    }

    fn write(&self, from: Channel, offset: u64, data: OoL) -> ServerResult<()> {
        /* TODO */
        Err(ErrorCode::NotImplemented)
    }
}

fn main() {
    println!("virtio: starting virtio driver...");
    let server = VirtioBlkServer::new();
    register_as!(&server, [blk_device]);
    serve_forever!(&server, [blk_device]);
}
