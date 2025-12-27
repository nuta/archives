#![no_std]

use ftl::channel::Channel;
use ftl::channel::Message;
use ftl::channel::MessageBuffer;
use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::prelude::*;
use ftl::spec::AppSpec;
use ftl::spec::EnvItem;
use ftl::spec::EnvType;

pub const SPEC: AppSpec = AppSpec {
    name: "echo_client",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[EnvItem {
        name: "echo_server",
        ty: EnvType::Service { name: "echo" },
    }],
    exports: &[],
};

#[derive(serde::Deserialize)]
struct Environ {
    echo_server: Channel,
}

#[derive(Debug)]
enum Context {
    Server,
}

fn main(env: Environ) {
    let server_ch = env.echo_server;

    server_ch
        .send(Message::Data { data: b"hello" })
        .expect("failed to send the first echo message");

    let mut eventloop = EventLoop::new().unwrap();
    eventloop.add_channel(server_ch, Context::Server).unwrap();

    let mut buffer = MessageBuffer::new();
    'mainloop: loop {
        let (ctx, event) = eventloop.wait().unwrap();
        match (ctx, event) {
            (Context::Server, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::Data { data }) => {
                        info!("received a reply: {:?}", core::str::from_utf8(data));
                        break 'mainloop;
                    }
                    Ok(msg) => {
                        warn!("unexpected message from server: {:?}", msg);
                    }
                    Err(e) => {
                        warn!("error receiving message from server: {:?}", e);
                    }
                }
            }
            (ctx, event) => {
                warn!("unexpected event: {:?} with context: {:?}", event, ctx);
            }
        }
    }
}
