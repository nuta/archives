#![no_std]

#[macro_use]
extern crate resea;
use core::cell::{RefCell};
use core::option::{Option};
use resea::arch::{ErrorCode};
use resea::channel::{Channel};
use resea::server::{ServerResult};
use resea::interfaces::kbd_device;
use resea::interfaces::kbd_device::{Server as KbdDeviceServer};
use resea::interfaces::rtc_device;
use resea::interfaces::rtc_device::{Server as RtcDeviceServer};
use resea::interfaces::events;
use resea::interfaces::events::{Server as EventsServer};
mod rtc;
mod keyboard;
use rtc::{Rtc};
use keyboard::{Keyboard};

struct PcServer {
    ch: Channel,
    kbd_irq_ch: Channel,
    kbd_listener: RefCell<Option<Channel>>,
    rtc: Rtc,
    kbd: Keyboard,
}

impl PcServer {
    pub fn new() -> PcServer {
        PcServer {
            ch: Channel::create(),
            kbd_irq_ch: Channel::create(),
            rtc: Rtc::new(),
            kbd: Keyboard::new(),
            kbd_listener: RefCell::new(None),
        }
    }

    pub fn listen_for_irq(&self) {
        self.kbd_irq_ch.transfer_to(&self.ch);
        self.kbd.listen_for_irq(&self.kbd_irq_ch);
    }
}

impl RtcDeviceServer for PcServer {
    fn read(&self, _from: Channel) -> ServerResult<(u32, u32, u32)> {
        let date = self.rtc.read();
        let ymd: u32 = ((date.year as u32) << 16) | ((date.month as u32) << 8) | date.day as u32;
        let hms: u32 = ((date.hour as u32) << 24) | ((date.min as u32) << 16) | ((date.sec as u32) << 8);
        let nsec = 0; // TODO:
        Ok((ymd, hms, nsec))
    }
}

impl KbdDeviceServer for PcServer {
    fn listen(&self, from: Channel, listener: Channel) -> ServerResult<()> {
        self.kbd_listener.replace(Some(listener));
        Ok(())
    }
}

impl EventsServer for PcServer {
    fn notification(&self, _from: Channel, notification: usize) -> ServerResult<()> {
        let keycode = self.kbd.get_keycode();
        // TODO: send to kbd_listener
        Err(ErrorCode::DontReply)
    }
}

fn main() {
    println!("pc: starting pc driver...");
    let server = PcServer::new();
    server.listen_for_irq();

    register_as!(&server, [kbd_device]);
    serve_forever!(&server, [kbd_device, events]);
}
