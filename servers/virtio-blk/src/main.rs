#![no_std]
#![feature(alloc)]

#[macro_use]
extern crate resea;

#[cfg(not(test))]
extern crate resea_langitems;

#[macro_use]
extern crate alloc;

extern crate virtio;
mod device;

use core::slice;
use device::{VirtioBlk, SECTOR_SIZE};
use resea::{ErrorCode, OoL, Channel, Result as ServerResult};
use resea::interfaces::blk_device;
use resea::interfaces::blk_device::Server as BlkDeviceServer;

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
        Ok(data)
    }

    fn write(&self, from: Channel, offset: u64, data: OoL) -> ServerResult<()> {
        /* TODO */
        Err(ErrorCode::NotImplemented as u8)
    }
}

fn main() {
    println!("virtio: starting virtio driver...");
    let server = VirtioBlkServer::new();
    register_as!(&server, [blk_device]);
    serve_forever!(&server, [blk_device]);
}
