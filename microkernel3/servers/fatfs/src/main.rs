#![no_std]
#![feature(alloc)]

#[macro_use]
extern crate resea;

#[cfg(not(test))]
extern crate resea_langitems;

#[macro_use]
extern crate alloc;

use resea::{Channel, ErrorCode, OoL, Result as ServerResult};
use resea::interfaces::blk_device::BlkDevice;
use resea::interfaces::fs;
use resea::interfaces::fs::Server as FsServer;
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
    fn open(&self, _from: Channel, _path: OoL) -> ServerResult<(isize)> {
        Err(ErrorCode::NotImplemented as u8)
    }

    fn open_dir(&self, _from: Channel, _path: OoL) -> ServerResult<(isize)> {
        Err(ErrorCode::NotImplemented as u8)
    }

    fn close(&self, _from: Channel, _fd: isize) -> ServerResult<()> {
        Err(ErrorCode::NotImplemented as u8)
    }

    fn read(&self, _from: Channel, _fd: isize, _offset: u64, _length: usize) -> ServerResult<(&[u8])> {
        Err(ErrorCode::NotImplemented as u8)
    }

    fn write(&self, _from: Channel, _fd: isize, _offset: u64, _data: OoL) -> ServerResult<()> {
        Err(ErrorCode::NotImplemented as u8)
    }

    fn readdir(&self, _from: Channel, _fd: isize, _offset: u64) -> ServerResult<(&[u8])> {
        Err(ErrorCode::NotImplemented as u8)
    }

    fn stat(&self, _from: Channel, _fd: isize, _path: OoL) -> ServerResult<(&[u8])> {
        Err(ErrorCode::NotImplemented as u8)
    }
}

fn main() {
    println!("fatfs: starting fatfs server...");
    let server = FatFsServer::new();

    println!("running...");
    use alloc::vec::Vec;
    let clus = server.fs.look_for_file("kernel.elf").unwrap();
    let mut buf = Vec::new();
    server.fs.read_file(&mut buf, clus, 0, 8);
    println!("==> {:?}", buf);

    register_as!(&server, [fs]);
    serve_forever!(&server, [fs]);
}
