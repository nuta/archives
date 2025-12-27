//! TCP/IP stack, in a single-threaded event loop.
//!
//! # Why not multi-threaded?
//!
//! Because smoltcp is not thread-safe. This means making this server
//! multi-threaded won't improve the performance (Amdahl's law).
//!
//! We plan to implement a multi-threaded, (almost) shared-nothing
//! TCP/IP stack in the future.
#![no_std]

use core::cmp::min;
use core::net::Ipv4Addr;
use core::ops::ControlFlow;

use ftl::channel::Channel;
use ftl::channel::MESSAGE_DATA_LEN_MAX;
use ftl::channel::Message;
use ftl::channel::MessageBuffer;
use ftl::collections::HashMap;
use ftl::collections::HashSet;
use ftl::error::ErrorCode;
use ftl::eventloop::Event;
use ftl::eventloop::EventLoop;
use ftl::handle::Handleable;
use ftl::poll::Readiness;
use ftl::prelude::*;
use ftl::rc::Rc;
use ftl::spec::AppSpec;
use ftl::spec::ExportItem;
use ftl::timer::Timer;
use smoltcp::iface::SocketHandle;
use smoltcp::wire::IpCidr;
use smoltcp::wire::IpEndpoint;
use smoltcp::wire::IpListenEndpoint;
use smoltcp::wire::Ipv4Cidr;

use crate::tcpip::TcpIp;
use crate::uri::Uri;

mod device;
mod tcpip;
mod time;
mod uri;

const GW_IP: Ipv4Addr = Ipv4Addr::new(10, 0, 2, 2);
const OUR_IP: IpCidr = IpCidr::Ipv4(Ipv4Cidr::new(Ipv4Addr::new(10, 0, 2, 15), 24));

pub const SPEC: AppSpec = AppSpec {
    name: "tcpip",
    start: |vsyscall| ftl::start(vsyscall, main),
    env: &[],
    exports: &[ExportItem::Service { name: "tcpip" }],
};

#[derive(serde::Deserialize)]
struct Environ {
    system_ch: Channel,
}

#[derive(Debug)]
enum Context {
    Startup,
    Driver,
    Timer,
    Control,
    TcpListen,
    TcpConn { handle: SocketHandle },
}

#[derive(Debug)]
enum Pcb {
    TcpListen {
        ch: Rc<Channel>,
        endpoint: IpListenEndpoint,
    },
    TcpConn {
        ch: Rc<Channel>,
    },
}

fn handle_device_open(
    eventloop: &mut EventLoop<Context>,
    tcpip: &mut TcpIp,
    mac: [u8; 6],
) -> Result<Channel, ErrorCode> {
    let Ok((our_ch, their_ch)) = Channel::new() else {
        warn!("failed to create channel");
        return Err(ErrorCode::TryLater);
    };

    let our_ch = Rc::new(our_ch);
    eventloop
        .add_channel(our_ch.clone(), Context::Driver)
        .unwrap();

    let transmit = move |data: &[u8]| {
        if let Err(e) = our_ch.send(Message::Data { data }) {
            warn!("failed to forward a packet to the driver: {:?}", e);
        }
    };

    tcpip.attach_driver(&mac, GW_IP, OUR_IP, transmit);
    Ok(their_ch)
}

fn handle_tcp_listen_open(
    tcpip: &mut TcpIp,
    eventloop: &mut EventLoop<Context>,
    pcbs: &mut HashMap<SocketHandle, Pcb>,
    host: &str,
    port: u16,
) -> Result<Channel, ErrorCode> {
    let addr: Ipv4Addr = match host.parse() {
        Ok(addr) => addr,
        Err(e) => {
            warn!("failed to parse IP address: {:?}", e);
            return Err(ErrorCode::InvalidUri);
        }
    };

    let endpoint = if addr.is_unspecified() {
        IpListenEndpoint { addr: None, port }
    } else {
        warn!("unsupported host: {}", host);
        return Err(ErrorCode::InvalidUri);
    };

    let handle = match tcpip.tcp_listen(endpoint) {
        Ok(handle) => handle,
        Err(e) => {
            warn!("failed to listen on {}:{}: {:?}", host, port, e);
            return Err(ErrorCode::InvalidUri);
        }
    };

    let Ok((our_ch, their_ch)) = Channel::new() else {
        warn!("failed to create channel");
        return Err(ErrorCode::TryLater);
    };

    let our_ch = Rc::new(our_ch);
    let our_ch2 = our_ch.clone();
    eventloop.add_channel(our_ch, Context::TcpListen).unwrap();
    pcbs.insert(
        handle,
        Pcb::TcpListen {
            ch: our_ch2,
            endpoint,
        },
    );

    Ok(their_ch)
}

fn on_tcp_accept(
    tcpip: &mut TcpIp,
    eventloop: &mut EventLoop<Context>,
    pcbs: &mut HashMap<SocketHandle, Pcb>,
    handle: SocketHandle,
    listen_ch: Rc<Channel>,
    listen_endpoint: IpListenEndpoint,
    remote_endpoint: IpEndpoint,
) {
    let Ok((our_ch, their_ch)) = Channel::new() else {
        warn!("failed to create channel");
        return;
    };

    let uri = format!("{}:{}", remote_endpoint.addr, remote_endpoint.port);

    let message = Message::Connect {
        ch: their_ch,
        uri: uri.as_bytes(),
    };

    if let Err(e) = listen_ch.send(message) {
        warn!("failed to forward an accepted TCP connection: {:?}", e);
        // We can't handle this channel. Remove it from the
        tcpip.tcp_close(handle);
        return;
    }

    let our_ch = Rc::new(our_ch);
    pcbs.insert(handle, Pcb::TcpConn { ch: our_ch.clone() });
    eventloop
        .add_channel(our_ch, Context::TcpConn { handle })
        .unwrap();

    let new_handle = tcpip.tcp_listen(listen_endpoint).unwrap();
    pcbs.insert(
        new_handle,
        Pcb::TcpListen {
            ch: listen_ch,
            endpoint: listen_endpoint,
        },
    );
}

#[derive(Debug, PartialEq, Eq)]
enum DrainResult {
    Ok,
    Backpressure,
}

/// Drains the channel into TCP write buffer.
///
/// It returns:
///
/// - `true`: The TCP write buffer is drained;
/// - `false`: The channel is full.
fn drain_ch_to_socket(
    tcpip: &mut TcpIp,
    buffer: &mut MessageBuffer,
    ch: &Channel,
    handle: SocketHandle,
) -> DrainResult {
    loop {
        if tcpip.tcp_writable_len(handle) < MESSAGE_DATA_LEN_MAX {
            return DrainResult::Backpressure;
        }

        let data = match ch.recv(buffer) {
            Ok(Message::Data { data }) => data,
            Ok(msg) => {
                warn!("unexpected message from TCP conn: {:?}", msg);
                continue;
            }
            Err(ErrorCode::Empty) => {
                // Successfully drained the channel.
                return DrainResult::Ok;
            }
            Err(ErrorCode::NoPeer) => {
                // The channel has been half-closed, that is, the peer
                // has written all data to the channel and closed it to
                // indicate the EOF.
                return DrainResult::Ok;
            }
            Err(e) => {
                warn!("error receiving message from TCP conn: {:?}", e);
                return DrainResult::Ok;
            }
        };

        if let Err(e) = tcpip.tcp_write(handle, data) {
            warn!("failed to write TCP data: {:?}", e);
        }
    }
}

/// Drains the TCP read buffer into the channel.
fn drain_socket_to_ch(
    tcpip: &mut TcpIp,
    eventloop: &mut EventLoop<Context>,
    handle: SocketHandle,
    ch: &Channel,
) -> DrainResult {
    let mut result = DrainResult::Ok;
    tcpip.tcp_read(handle, |data: &[u8]| {
        let data = &data[..min(data.len(), MESSAGE_DATA_LEN_MAX)];
        match ch.send(Message::Data { data }) {
            Ok(()) => data.len(),
            Err(ErrorCode::Backpressure) => {
                debug!("data channel is full (backpressure)");
                eventloop
                    .listen(ch.handle_id(), Readiness::WRITABLE)
                    .unwrap();
                result = DrainResult::Backpressure;
                0
            }
            Err(ErrorCode::NoPeer) => {
                warn!("TCP read closed");
                0
            }
            Err(e) => {
                warn!("failed to send TCP read data: {:?}", e);
                0
            }
        }
    });

    result
}

fn main(env: Environ) {
    let mut eventloop = EventLoop::new().unwrap();

    let system_ch = env.system_ch;
    eventloop.add_channel(system_ch, Context::Startup).unwrap();

    let timer = Rc::new(Timer::new().unwrap());
    eventloop.add_timer(timer.clone(), Context::Timer).unwrap();

    // Backpressured sockets: conn channel -> TCP write buffer
    let mut write_blocked_sockets = HashSet::new();
    // Backpressured sockets: TCP read buffer -> conn channel
    let mut read_blocked_sockets = HashSet::new();
    // Sockets being closed and still have data to drain
    let mut pending_close_sockets = HashSet::new();

    let mut tcpip = TcpIp::new();
    let mut pcbs = HashMap::new();
    let mut msgbuffer = MessageBuffer::new();
    loop {
        // Poll the TCP/IP stack.
        loop {
            let mut did_work = false;
            let should_break = match tcpip.poll() {
                ControlFlow::Continue(changes) => {
                    for (handle, change) in changes {
                        let pcb = pcbs.get(&handle).unwrap();
                        match (change, pcb) {
                            (
                                tcpip::Change::TcpAccepted {
                                    endpoint: remote_endpoint,
                                },
                                Pcb::TcpListen {
                                    ch: tx,
                                    endpoint: listen_endpoint,
                                },
                            ) => {
                                let tx = tx.clone();
                                let listen_endpoint = *listen_endpoint;
                                on_tcp_accept(
                                    &mut tcpip,
                                    &mut eventloop,
                                    &mut pcbs,
                                    handle,
                                    tx,
                                    listen_endpoint,
                                    remote_endpoint,
                                );
                                did_work = true;
                            }
                            (tcpip::Change::TcpAccepted { .. }, Pcb::TcpConn { .. }) => {}
                            (tcpip::Change::TcpReadable, Pcb::TcpConn { ch: tx }) => {
                                // Skip if socket is blocked waiting for channel to be writable
                                if !read_blocked_sockets.contains(&handle) {
                                    if drain_socket_to_ch(&mut tcpip, &mut eventloop, handle, tx)
                                        == DrainResult::Ok
                                    {
                                        did_work = true;
                                    } else {
                                        read_blocked_sockets.insert(handle);
                                    }
                                }
                            }
                            (
                                tcpip::Change::TcpClosed,
                                Pcb::TcpConn { ch, .. } | Pcb::TcpListen { ch, .. },
                            ) => {
                                eventloop.remove(ch.handle_id()).unwrap();
                                tcpip.tcp_remove(handle);
                                pcbs.remove(&handle);
                                write_blocked_sockets.remove(&handle);
                                read_blocked_sockets.remove(&handle);
                                pending_close_sockets.remove(&handle);
                                did_work = true;
                            }
                            (change, pcb) => {
                                unreachable!("unexpected socket change: {:?}, {:?}", change, pcb);
                            }
                        }
                    }
                    // Break if we didn't actually do any work (all sockets blocked)
                    !did_work
                }
                ControlFlow::Break(deadline) => {
                    if let Some(deadline) = deadline {
                        timer.set(deadline).unwrap();
                    }
                    true
                }
            };

            // Retry backpressured sockets - do this BEFORE breaking so we don't
            // miss retrying when poll returns Break.
            let mut made_progress = false;
            for handle in core::mem::take(&mut write_blocked_sockets) {
                let pcb = pcbs.get(&handle).unwrap();
                if let Pcb::TcpConn { ch } = pcb {
                    if drain_ch_to_socket(&mut tcpip, &mut msgbuffer, ch, handle) == DrainResult::Ok
                    {
                        made_progress = true;
                        if pending_close_sockets.remove(&handle) {
                            // Channel was closed and now fully drained, close TCP
                            tcpip.tcp_close(handle);
                        } else {
                            // Channel still open, re-listen for new messages.
                            eventloop
                                .listen(ch.handle_id(), Readiness::READABLE)
                                .unwrap();
                        }
                    } else {
                        // TCP write buffer is still full, retry later.
                        write_blocked_sockets.insert(handle);
                    }
                }
            }

            // Only break if poll said to break AND we made no progress on backpressure
            if should_break && !made_progress {
                break;
            }
        }

        // Handle events.
        let (ctx, event) = eventloop.wait().unwrap();
        match (ctx, event) {
            (Context::Startup, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut msgbuffer) {
                    Ok(Message::Connect { ch: new_ch, uri: _ }) => {
                        eventloop.add_channel(new_ch, Context::Control).unwrap();
                    }
                    Ok(msg) => {
                        warn!("unexpected message from system: {:?}", msg);
                    }
                    Err(e) => {
                        warn!("error receiving message from system: {:?}", e);
                    }
                }
            }
            (Context::Driver, Event::ChannelReadable { ch }) => {
                match ch.recv(&mut msgbuffer) {
                    Ok(Message::Data { data }) => {
                        tcpip.receive_packet(data);
                    }
                    Ok(msg) => {
                        warn!("unexpected message from driver: {:?}", msg);
                    }
                    Err(e) => {
                        warn!("error receiving message from driver: {:?}", e);
                    }
                }
            }
            (Context::Timer, Event::TimerFired { .. }) => {
                // Nothing to do here. `flush()` will let Smoltcp make progress.
            }
            (Context::Control, Event::ChannelReadable { ch }) => {
                let ch = ch.clone();
                let (cookie, uri) = match ch.recv(&mut msgbuffer) {
                    Ok(Message::Open { cookie, uri }) => (cookie, uri),
                    Ok(msg) => {
                        warn!("unexpected message from control: {:?}", msg);
                        continue;
                    }
                    Err(e) => {
                        warn!("error receiving message from control: {:?}", e);
                        continue;
                    }
                };

                let result = match Uri::parse(uri) {
                    Ok(Uri::Device { mac }) => handle_device_open(&mut eventloop, &mut tcpip, mac),
                    Ok(Uri::TcpListen { host, port }) => {
                        handle_tcp_listen_open(&mut tcpip, &mut eventloop, &mut pcbs, host, port)
                    }
                    Err(e) => {
                        warn!("failed to parse URI: {:?}: {:?}", str::from_utf8(uri), e);
                        Err(ErrorCode::InvalidUri)
                    }
                };

                let reply = match result {
                    Ok(new_ch) => Message::OpenReply { cookie, ch: new_ch },
                    Err(error) => Message::ErrorReply { cookie, error },
                };

                if let Err(e) = ch.send(reply) {
                    warn!("failed to reply to open: {:?}", e);
                }
            }
            (Context::Control, Event::ChannelClosed { ch }) => {
                let id = ch.handle_id();
                eventloop.remove(id).unwrap();
            }
            (Context::TcpConn { handle, .. }, Event::ChannelWritable { ch }) => {
                // Channel is writable again after backpressure, continue reading from TCP
                let handle = *handle;
                let ch = ch.clone();
                read_blocked_sockets.remove(&handle);
                if drain_socket_to_ch(&mut tcpip, &mut eventloop, handle, &ch)
                    == DrainResult::Backpressure
                {
                    // Still blocked, put it back
                    read_blocked_sockets.insert(handle);
                }
            }
            (Context::TcpConn { handle, .. }, Event::ChannelReadable { ch }) => {
                let handle = *handle;
                if drain_ch_to_socket(&mut tcpip, &mut msgbuffer, ch, handle)
                    == DrainResult::Backpressure
                {
                    trace!("TCP write buffer is full, stopping to receive more messages");
                    let id = ch.handle_id();
                    eventloop.unlisten(id, Readiness::READABLE).unwrap();
                    write_blocked_sockets.insert(handle);
                }
            }
            (Context::TcpConn { handle, .. }, Event::ChannelClosed { ch }) => {
                read_blocked_sockets.remove(handle);
                if drain_ch_to_socket(&mut tcpip, &mut msgbuffer, ch, *handle) == DrainResult::Ok {
                    // All data drained, close now.
                    write_blocked_sockets.remove(handle);
                    tcpip.tcp_close(*handle);
                } else {
                    // TCP write buffer full, can't drain yet. Retry later.
                    write_blocked_sockets.insert(*handle);
                    pending_close_sockets.insert(*handle);
                }
            }
            (ctx, event) => {
                debug!("unexpected context: {:?} with event: {:?}", ctx, event);
            }
        }
    }
}
