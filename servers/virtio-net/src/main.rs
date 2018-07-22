#![feature(alloc)]
#![no_std]

#[macro_use]
extern crate alloc;
#[macro_use]
extern crate resea;

#[cfg(not(test))]
extern crate resea_langitems;

extern crate virtio;
mod device;

use core::option::Option;
use core::cell::RefCell;
use resea::{Channel, OoL, Result as ServerResult};
use resea::arch::x64::{Irq};
use resea::interfaces::net_device;
use resea::interfaces::net_device::{Server as NetDeviceServer};
use resea::interfaces::events;
use resea::interfaces::events::{Server as EventsServer};
use device::{VirtioNet};

struct VirtioNetServer {
    device: VirtioNet,
    ch: Channel,
    listener: RefCell<Option<Channel>>,
}

impl VirtioNetServer {
    pub fn new() -> VirtioNetServer {
        let server = Channel::create();
        let irq_ch = Channel::create();

        let irq = Irq::new(11 /* XXX: */);
        irq_ch.transfer_to(&server);
        irq.listen(&irq_ch);

        VirtioNetServer {
            device: VirtioNet::new(0xc040),
            ch: server,
            listener: RefCell::new(None),
        }
    }
}

impl NetDeviceServer for VirtioNetServer {
    fn send(&self, _from: Channel, data: OoL) -> ServerResult<()> {
        self.device.send(data.as_slice()).unwrap();
        Ok(())
    }

    fn get_hwaddr(&self, _from: Channel) -> ServerResult<(&[u8])> {
        Ok(self.device.get_hwaddr())
    }

    fn listen(&self, _from: Channel, listener: Channel) -> ServerResult<()> {
        self.listener.replace(Some(listener));
        Ok(())
    }

    fn received(&self, _from: Channel, _data: OoL) {
    }
 }

impl EventsServer for VirtioNetServer {
    fn notification(&self, _from: Channel, _notification: usize) {
        if let Some(payload) = self.device.recv() {
            if let Some(ref listener) = *self.listener.borrow() {
                net_device::NetDevice::from_channel(listener).received(payload).unwrap();
            }
        }
    }
}

fn main() {
    println!("virtio: starting virtio driver...");
    let server = VirtioNetServer::new();
    register_as!(&server, [net_device]);
    serve_forever!(&server, [net_device, events]);
}
