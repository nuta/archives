#![no_std]

mod endpoints;
mod http;

use ftl::channel::Channel;
use ftl::channel::Cookie;
use ftl::channel::Message;
use ftl::channel::MessageBuffer;
use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::handle::Handleable;
use ftl::prelude::*;
use ftl::rc::Rc;
use ftl::spec::AppSpec;
use ftl::spec::EnvItem;
use ftl::spec::EnvType;
use ftl::thread::Thread;
use ftl::timer::Instant;
use serde::Deserialize;

use crate::http::HeaderName;
use crate::http::ResponseWriter;
use crate::http::ResponseWriterImpl;

pub const SPEC: AppSpec = AppSpec {
    name: "apiserver",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[EnvItem {
        name: "tcpip",
        ty: EnvType::Service { name: "tcpip" },
    }],
    exports: &[],
};

#[derive(Deserialize)]
struct Environ {
    pub tcpip: Channel,
}

#[derive(Debug)]
enum Context {
    TcpIp,
    Listen,
    ConnThread,
}

pub(crate) static STARTED_AT: spin::Lazy<Instant> = spin::Lazy::new(ftl::timer::now);
const LISTEN_COOKIE: Cookie = Cookie::new(1);

fn main(env: Environ) {
    spin::Lazy::force(&STARTED_AT);

    env.tcpip
        .send(Message::Open {
            cookie: LISTEN_COOKIE,
            uri: b"tcp-listen:0.0.0.0:80",
        })
        .unwrap();

    let mut buffer = MessageBuffer::new();

    let mut eventloop = EventLoop::new().unwrap();
    eventloop.add_channel(env.tcpip, Context::TcpIp).unwrap();

    info!("listening on TCP port 80");

    loop {
        let (ctx, event) = eventloop.wait().unwrap();
        match (ctx, event) {
            (Context::TcpIp, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::OpenReply { ch: new_ch, cookie }) if cookie == LISTEN_COOKIE => {
                        eventloop.add_channel(new_ch, Context::Listen).unwrap();
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
                }
            }
            (Context::Listen, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut buffer) {
                    Ok(Message::Connect { ch, .. }) => {
                        let thread = Thread::spawn(move || worker_thread(ch)).unwrap();
                        eventloop.add_thread(thread, Context::ConnThread).unwrap();
                    }
                    Ok(msg) => {
                        warn!("unexpected message from TCP/IP: {:?}", msg);
                    }
                    Err(e) => {
                        panic!("failed to receive connection from TCP/IP: {:?}", e);
                    }
                }
            }
            (Context::ConnThread, Event::ThreadExited { thread }) => {
                trace!("connection thread exited");
                let id = thread.handle_id();
                eventloop.remove(id).unwrap();
            }
            (ctx, event) => {
                warn!("unexpected event: {:?} with context: {:?}", event, ctx);
            }
        }
    }
}

/// A per-connection thread that handles a TCP connection.
fn worker_thread(conn_ch: Channel) {
    let mut buffer = MessageBuffer::new();
    let mut parser = http::RequestParser::new();

    let conn_ch = Rc::new(conn_ch);

    let mut eventloop = EventLoop::new().unwrap();
    eventloop.add_channel(conn_ch.clone(), ()).unwrap();
    loop {
        let (_, event) = eventloop.wait().unwrap();
        let data = match event {
            Event::ChannelReadable { ch } => {
                match ch.recv(&mut buffer) {
                    Ok(Message::Data { data }) => data,
                    Ok(msg) => {
                        warn!("unexpected message from connection channel: {:?}", msg);
                        continue;
                    }
                    Err(e) => {
                        error!("failed to receive data from connection channel: {:?}", e);
                        break;
                    }
                }
            }
            Event::ChannelClosed { .. } => {
                trace!("connection channel closed");
                break;
            }
            event => {
                warn!("unexpected event: {:?}", event);
                break;
            }
        };

        // Note: parse_chunk returns `Ok(Some(Request))` multiple times
        // in keep-alive connections.
        match parser.parse_chunk(data) {
            Ok(Some(request)) => {
                trace!("{} {}", request.method, request.path);

                let mut resp = ResponseWriterImpl::new(conn_ch.clone());
                resp.headers_mut()
                    .insert(HeaderName::X_POWERED_BY, "ftl")
                    .unwrap();

                endpoints::route(&request, &mut resp);
                resp.end();
            }
            Ok(None) => {
                continue;
            }
            Err(e) => {
                error!("failed to parse request: {:?}", e);
                break;
            }
        }
    }
}
