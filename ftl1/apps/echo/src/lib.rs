#![no_std]

extern crate alloc;

use ftl::channel::Channel;
use ftl::channel::Message;
use ftl::channel::MessageBuffer;
use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::handle::Handleable;
use ftl::prelude::*;
use ftl::spec::AppSpec;
use ftl::spec::ExportItem;

pub const SPEC: AppSpec = AppSpec {
    name: "echo",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[],
    exports: &[ExportItem::Service { name: "echo" }],
};

#[derive(serde::Deserialize)]
struct Environ {
    system_ch: Channel,
}

#[derive(Debug)]
enum Context {
    System,
    Client,
}

fn main(env: Environ) {
    let mut eventloop = EventLoop::new().unwrap();
    eventloop
        .add_channel(env.system_ch, Context::System)
        .unwrap();

    let mut buffer = MessageBuffer::new();
    loop {
        match eventloop.wait().unwrap() {
            (Context::System, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::Connect { ch: new_ch, .. }) => {
                        eventloop.add_channel(new_ch, Context::Client).unwrap();
                    }
                    Ok(msg) => {
                        warn!("unexpected message from system: {:?}", msg);
                    }
                    Err(e) => {
                        panic!("error receiving message from system: {:?}", e);
                    }
                }
            }
            (Context::Client, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::Data { data }) => {
                        info!("echoing back {:?}", str::from_utf8(data));
                        if let Err(e) = ch.send(Message::Data { data }) {
                            warn!("failed to reply: {:?}", e);
                        }
                    }
                    Ok(msg) => {
                        warn!("unexpected message from client: {:?}", msg);
                    }
                    Err(e) => {
                        panic!("error receiving message from client: {:?}", e);
                    }
                }
            }
            (Context::Client, Event::ChannelClosed { ch }) => {
                let id = ch.handle_id();
                eventloop.remove(id).unwrap();
            }
            (ctx, event) => {
                warn!("unexpected event: {:?} with context: {:?}", event, ctx);
            }
        }
    }
}
