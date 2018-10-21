#![no_std]
#![feature(alloc)]

#[macro_use]
extern crate resea;

#[cfg(not(test))]
extern crate resea_langitems;

#[macro_use]
extern crate alloc;

use core::cell::RefCell;
use resea::{Channel, Result as ServerResult};
use resea::interfaces::events;
use resea::interfaces::events::Server as EventsServer;
use resea::interfaces::kbd_device;
use resea::interfaces::kbd_device::Server as KbdDeviceServer;
use resea::interfaces::rtc_device::Server as RtcDeviceServer;
mod keyboard;
mod rtc;
use keyboard::{Keyboard, KeyEvent};
use rtc::Rtc;

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
        let hms: u32 =
            ((date.hour as u32) << 24) | ((date.min as u32) << 16) | ((date.sec as u32) << 8);
        let nsec = 0; // TODO:
        Ok((ymd, hms, nsec))
    }
}

impl KbdDeviceServer for PcServer {
    fn listen(&self, _from: Channel, listener: Channel) -> ServerResult<()> {
        self.kbd_listener.replace(Some(listener));
        Ok(())
    }

    fn keydown(&self, _from: Channel, _keycode: u16) {
    }

    fn keyup(&self, _from: Channel, _keycode: u16) {
    }
}

impl EventsServer for PcServer {
    fn notification(&self, _from: Channel, _notification: usize) {
        let keycode = self.kbd.get_keycode();
        if let Some(ref ch) = *self.kbd_listener.borrow() {
            let listener = kbd_device::KbdDevice::from_channel(&ch);
            match keycode {
                KeyEvent::KeyDown(code) => listener.keydown(code).unwrap(),
                KeyEvent::KeyUp(code) => listener.keyup(code).unwrap(),
            }
        }
    }
}

fn main() {
    println!("pc: starting pc driver...");
    let server = PcServer::new();
    server.listen_for_irq();

    register_as!(&server, [kbd_device]);
    serve_forever!(&server, [kbd_device, events]);
}
