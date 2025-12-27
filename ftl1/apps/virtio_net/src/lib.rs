#![no_std]

extern crate alloc;

use alloc::format;

use ftl::channel::Channel;
use ftl::channel::Cookie;
use ftl::channel::Message;
use ftl::channel::MessageBuffer;
use ftl::error::ErrorCode;
use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::interrupt::Interrupt;
use ftl::prelude::*;
use ftl::rc::Rc;
use ftl::spec::AppSpec;
use ftl::spec::DeviceMatcher;
use ftl::spec::EnvItem;
use ftl::spec::EnvType;
use ftl_driver::device::DeviceDesc;

use crate::virtio_net::VirtioNet;

mod virtio_net;

pub const SPEC: AppSpec = AppSpec {
    name: "virtio_net",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[
        EnvItem {
            name: "device",
            ty: EnvType::Device {
                matcher: DeviceMatcher::DeviceTree {
                    compatible: "virtio,mmio",
                },
            },
        },
        EnvItem {
            name: "tcpip",
            ty: EnvType::Service { name: "tcpip" },
        },
    ],
    exports: &[],
};

#[derive(serde::Deserialize)]
struct Environ {
    tcpip: Channel,
    device: DeviceDesc,
}

fn probe(device: &DeviceDesc) -> (VirtioNet, Interrupt) {
    for m in &device.matches {
        match VirtioNet::probe(m.iospace, m.dma) {
            Ok(virtio_net) => {
                let interrupt = Interrupt::acquire(m.irq).unwrap();
                return (virtio_net, interrupt);
            }
            Err(virtio_net::Error::OtherDeviceType(_)) => {
                continue;
            }
            Err(e) => {
                panic!("failed to probe virtio_net: {:?}", e);
            }
        }
    }

    panic!("probe failed");
}

#[derive(Debug)]
enum Context {
    TcpIp,
    Packet,
    Interrupt,
}

const OPEN_COOKIE: Cookie = Cookie::new(1);

fn main(env: Environ) {
    let (mut device, interrupt) = probe(&env.device);

    let mac = device.mac_address();
    let uri = format!(
        "ethernet:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );

    env.tcpip
        .send(Message::Open {
            cookie: OPEN_COOKIE,
            uri: uri.as_bytes(),
        })
        .expect("failed to open the TCP/IP server");

    let mut buffer = MessageBuffer::new();

    let mut eventloop = EventLoop::new().unwrap();
    eventloop
        .add_interrupt(interrupt, Context::Interrupt)
        .unwrap();

    eventloop.add_channel(env.tcpip, Context::TcpIp).unwrap();

    let mut packet_ch = None;
    loop {
        let (ctx, event) = eventloop.wait().unwrap();
        match (ctx, event) {
            (Context::TcpIp, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::OpenReply { ch, cookie }) if cookie == OPEN_COOKIE => {
                        let ch = Rc::new(ch);
                        packet_ch = Some(ch.clone());
                        eventloop.add_channel(ch, Context::Packet).unwrap();
                    }
                    Ok(Message::OpenReply { cookie, .. }) => {
                        panic!("unexpected cookie in open reply: {:?}", cookie);
                    }
                    Ok(m) => {
                        panic!("unexpected message from TCP/IP: {:?}", m);
                    }
                    Err(e) => {
                        panic!("failed to receive reply from TCP/IP: {:?}", e);
                    }
                };
            }
            (Context::Packet, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::Data { data }) => {
                        if let Err(e) = device.send_packet(data) {
                            warn!("failed to send packet: {:?}", e);
                        }
                    }
                    Ok(m) => {
                        panic!("unexpected message from TCP/IP: {:?}", m);
                    }
                    Err(e) => {
                        panic!("failed to receive packet from TCP/IP: {:?}", e);
                    }
                }
            }
            (Context::Interrupt, Event::InterruptFired { interrupt }) => {
                if let Some(packet_ch) = &packet_ch {
                    let on_receive = |pkt: &[u8]| {
                        match packet_ch.send(Message::Data { data: pkt }) {
                            Ok(()) => (),
                            Err(ErrorCode::Backpressure) => {
                                trace!("the channel is full, drop the packet");
                            }
                            Err(e) => {
                                panic!("failed to send packet to TCP/IP: {:?}", e);
                            }
                        }
                    };

                    if let Err(e) = device.handle_irq(on_receive) {
                        panic!("failed to handle interrupt: {:?}", e);
                    }

                    if let Err(e) = interrupt.acknowledge() {
                        panic!("failed to acknowledge interrupt: {:?}", e);
                    }
                }
            }
            (ctx, event) => {
                warn!("unexpected event: {:?} with context: {:?}", event, ctx);
            }
        }
    }
}
