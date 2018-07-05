#![no_std]

#[macro_use]
extern crate resea;
use resea::channel::{Channel};
use resea::server::{ServerResult};
use resea::interfaces::rtc_device;
use resea::interfaces::rtc_device::{Server as RtcServer};
mod rtc;
use rtc::{Rtc};

struct PcServer {
    ch: Channel,
    rtc: Rtc,
}

impl PcServer {
    pub fn new() -> PcServer {
        PcServer {
            ch: Channel::create(),
            rtc: Rtc::new(),
        }
    }
}

impl RtcServer for PcServer {
    fn read(&self, _from: Channel) -> ServerResult<(u32, u32, u32)> {
        let date = self.rtc.read();
        let ymd: u32 = ((date.year as u32) << 16) | ((date.month as u32) << 8) | date.day as u32;
        let hms: u32 = ((date.hour as u32) << 24) | ((date.min as u32) << 16) | ((date.sec as u32) << 8);
        let nsec = 0; // TODO:
        Ok((ymd, hms, nsec))
    }
}

fn main() {
    println!("rtc: starting rtc driver...");
    let server = PcServer::new();
    register_as!(&server, rtc_device);
    serve_forever!(&server, rtc_device);
}
