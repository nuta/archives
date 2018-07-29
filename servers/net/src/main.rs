#![feature(alloc)]
#![no_std]

#[macro_use]
extern crate alloc;
#[macro_use]
extern crate resea;
extern crate netstack;
#[cfg(not(test))]
extern crate resea_langitems;

use netstack::{IpAddr, Ipv4Addr, Stack};
use resea::interfaces::net;
use resea::interfaces::net::Server as NetServer;
use resea::interfaces::net_device::NetDevice;
use resea::{Channel, ErrorCode, OoL, Result as ServerResult};

struct Server {
    ch: Channel,
    stack: Stack,
}

impl Server {
    pub fn new() -> Server {
        let server = Channel::create();
        let device = NetDevice::connect();
        let stack = Stack::new();

        let listener = Channel::create();
        listener.transfer_to(&server);
        device.listen(listener).expect("net_device.listen error");

        stack.add_ethernet_device(device, true);

        Server {
            ch: server,
            stack: stack,
        }
    }
}

impl NetServer for Server {
    fn open_udp(&self, _from: Channel, _addr: OoL, _flags: u32) -> ServerResult<(usize)> {
        Err(ErrorCode::NotImplemented.into())
    }

    fn close(&self, _from: Channel, _sock: usize) -> ServerResult<()> {
        Err(ErrorCode::NotImplemented.into())
    }
}

mod net_device_listener {
    use resea::interfaces::net_device;
    use resea::{Channel, ErrorCode, Header, HeaderTrait, OoL, Payload};
    pub const SERVICE_ID: u16 = net_device::SERVICE_ID;

    pub trait Server {
        fn received(&self, from: Channel, data: OoL);
    }

    impl Server {
        pub fn handle(
            &self,
            from: Channel,
            header: Header,
            a0: Payload,
            a1: Payload,
            _a2: Payload,
            _a3: Payload,
        ) -> (Header, Payload, Payload, Payload, Payload) {
            match header.msg_type() {
                net_device::NET_DEVICE_RECEIVED_MSG => {
                    self.received(from, OoL::from_payload(a0 as usize, a1 as usize));
                    (ErrorCode::DontReply as u64, 0, 0, 0, 0)
                }
                _ => (ErrorCode::DontReply as u64, 0, 0, 0, 0),
            }
        }
    }
}

impl net_device_listener::Server for Server {
    fn received(&self, _from: Channel, data: OoL) {
        println!("received a payload from net_device!");
        self.stack.receive_frame(data.as_slice()).ok();
        self.stack.send_queued_packets();
    }
}

fn main() {
    println!("net: starting net server...");
    let server = Server::new();
    register_as!(&server, [net]);
    println!(">>>>>>>>>>>>> server-forever!");
    server.stack.send_queued_packets();
    serve_forever!(&server, [net, net_device_listener]);
}
