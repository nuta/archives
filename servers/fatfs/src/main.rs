#![no_std]

#[macro_use]
extern crate resea;

use resea::arch::{ErrorCode, OoL};
use resea::channel::Channel;
use resea::interfaces::blk_device::BlkDevice;
use resea::interfaces::fs;
use resea::interfaces::fs::Server as FsServer;
use resea::server::ServerResult;
mod fat;
use fat::Fat;

struct FatFsServer {
    ch: Channel,
    fs: Fat,
}

impl FatFsServer {
    pub fn new() -> FatFsServer {
        let blk = BlkDevice::connect();
        let fs = Fat::new(blk, 0x200 /* FIXME: properly recognize partitions */);

        FatFsServer {
            ch: Channel::create(),
            fs: fs,
        }
    }
}

impl FsServer for FatFsServer {
    fn open(&self, from: Channel, path: OoL) -> ServerResult<(isize)> {
        Err(ErrorCode::NotImplemented)
    }

    fn open_dir(&self, from: Channel, path: OoL) -> ServerResult<(isize)> {
        Err(ErrorCode::NotImplemented)
    }

    fn close(&self, from: Channel, fd: isize) -> ServerResult<()> {
        Err(ErrorCode::NotImplemented)
    }

    fn read(&self, from: Channel, fd: isize, offset: u64, length: usize) -> ServerResult<(&[u8])> {
        Err(ErrorCode::NotImplemented)
    }

    fn write(&self, from: Channel, fd: isize, offset: u64, data: OoL) -> ServerResult<()> {
        Err(ErrorCode::NotImplemented)
    }

    fn readdir(&self, from: Channel, fd: isize, offset: u64) -> ServerResult<(&[u8])> {
        Err(ErrorCode::NotImplemented)
    }

    fn stat(&self, from: Channel, fd: isize, path: OoL) -> ServerResult<(&[u8])> {
        Err(ErrorCode::NotImplemented)
    }
}

fn main() {
    println!("fatfs: starting fatfs server...");
    let server = FatFsServer::new();
    register_as!(&server, [fs]);
    serve_forever!(&server, [fs]);
}
