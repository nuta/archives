use core::net::Ipv4Addr;
use core::ops::ControlFlow;

use ftl::collections::HashSet;
use ftl::prelude::*;
use ftl::timer::Instant;
use smoltcp::iface::Interface;
use smoltcp::iface::PollResult;
use smoltcp::iface::SocketHandle;
use smoltcp::iface::SocketSet;
use smoltcp::socket::tcp;
use smoltcp::socket::tcp::ListenError;
use smoltcp::wire::EthernetAddress;
use smoltcp::wire::HardwareAddress;
use smoltcp::wire::IpCidr;
use smoltcp::wire::IpEndpoint;
use smoltcp::wire::IpListenEndpoint;

use crate::device::Device;
use crate::time::SmolInstant;

const TCP_BUFFER_SIZE: usize = 8192;

#[derive(Debug)]
pub enum Error {
    UnsupportedHost,
    InvalidState,
    PartialWrite,
}

#[derive(Debug)]
#[allow(clippy::enum_variant_names)]
pub enum Change {
    TcpReadable,
    TcpAccepted {
        // The remote endpoint of the accepted socket.
        endpoint: IpEndpoint,
    },
    TcpClosed,
}

pub struct Driver {
    device: Device,
    iface: Interface,
}

pub struct TcpIp {
    instant: SmolInstant,
    driver: Option<Driver>,
    sockets: SocketSet<'static>,
    /// Sockets that have transitioned from State::Listen to State::SynReceived.
    ///
    /// This is to avoid emitting TcpAccepted multiple times.
    accepted_sockets: HashSet<SocketHandle>,
}

impl TcpIp {
    pub fn new() -> Self {
        Self {
            instant: SmolInstant::new(),
            driver: None,
            sockets: SocketSet::new(Vec::with_capacity(4)),
            accepted_sockets: HashSet::new(),
        }
    }

    pub fn attach_driver(
        &mut self,
        hwaddr: &[u8; 6],
        gw_ip: Ipv4Addr,
        our_ip: IpCidr,
        transmit: impl Fn(&[u8]) + 'static,
    ) {
        let hwaddr = HardwareAddress::Ethernet(EthernetAddress::from_bytes(hwaddr));
        let config = smoltcp::iface::Config::new(hwaddr);
        let mut device = Device::new(Box::new(transmit));
        let mut iface = Interface::new(config, &mut device, self.instant.now());
        iface.routes_mut().add_default_ipv4_route(gw_ip).unwrap();
        iface.update_ip_addrs(|ip_addrs| {
            ip_addrs.push(our_ip).unwrap();
        });

        self.driver = Some(Driver { device, iface });
    }

    pub fn receive_packet(&mut self, packet: &[u8]) {
        let Some(Driver { device, .. }) = self.driver.as_mut() else {
            panic!("driver not attached");
        };

        device.receive_packet(packet);
    }

    pub fn tcp_write(&mut self, handle: SocketHandle, data: &[u8]) -> Result<(), Error> {
        let socket = self.sockets.get_mut::<tcp::Socket>(handle);
        let written_len = socket.send_slice(data).map_err(|_| Error::InvalidState)?;
        if written_len != data.len() {
            return Err(Error::PartialWrite);
        }

        Ok(())
    }

    pub fn tcp_writable_len(&self, handle: SocketHandle) -> usize {
        let socket = self.sockets.get::<tcp::Socket>(handle);
        socket.send_capacity() - socket.send_queue()
    }

    pub fn tcp_read<F>(&mut self, handle: SocketHandle, mut f: F)
    where
        F: FnMut(&[u8]) -> usize,
    {
        let socket = self.sockets.get_mut::<tcp::Socket>(handle);
        while socket.can_recv() {
            let result = socket.recv(|data| {
                let read_len = f(data);
                (read_len, read_len)
            });

            match result {
                Ok(0) => {
                    break;
                }
                Ok(_) => {
                    continue;
                }
                Err(e) => {
                    unreachable!("unexpected socket recv error: {:?}", e);
                }
            }
        }
    }

    pub fn tcp_listen(&mut self, endpoint: IpListenEndpoint) -> Result<SocketHandle, Error> {
        let rx_buf = tcp::SocketBuffer::new(vec![0; TCP_BUFFER_SIZE]);
        let tx_buf = tcp::SocketBuffer::new(vec![0; TCP_BUFFER_SIZE]);
        let mut socket = tcp::Socket::new(rx_buf, tx_buf);

        // Disable Nagle's Algorithm by default. Add an option if you need it.
        socket.set_nagle_enabled(false);

        socket.set_ack_delay(None);

        match socket.listen(endpoint) {
            Ok(_) => {}
            Err(ListenError::Unaddressable) => {
                return Err(Error::UnsupportedHost);
            }
            Err(e) => {
                unreachable!("unexpected listen error: {:?}", e);
            }
        }

        let handle = self.sockets.add(socket);
        Ok(handle)
    }

    pub fn tcp_close(&mut self, handle: SocketHandle) {
        let socket = self.sockets.get_mut::<tcp::Socket>(handle);
        socket.close();
    }

    pub fn tcp_remove(&mut self, handle: SocketHandle) {
        self.sockets.remove(handle);
    }

    pub fn poll(&mut self) -> ControlFlow<Option<Instant>, Vec<(SocketHandle, Change)>> {
        use smoltcp::socket::Socket;
        use smoltcp::socket::tcp::State;

        let Some(Driver { device, iface }) = self.driver.as_mut() else {
            // The driver is not yet attached.
            return ControlFlow::Break(None);
        };

        let timestamp = self.instant.now();
        let poll_result = iface.poll(timestamp, device, &mut self.sockets);

        let mut changes = Vec::new();
        for (handle, socket) in self.sockets.iter_mut() {
            match socket {
                Socket::Tcp(socket) => {
                    match socket.state() {
                        State::Established | State::FinWait1 | State::FinWait2 => {
                            if socket.can_recv() {
                                changes.push((handle, Change::TcpReadable));
                            }
                        }
                        State::CloseWait => {
                            if socket.can_recv() {
                                changes.push((handle, Change::TcpReadable));
                            } else {
                                socket.close();
                            }
                        }
                        State::Closing | State::LastAck => {
                            // Still waiting for ACKs, don't remove yet.
                        }
                        State::TimeWait | State::Closed => {
                            debug_assert!(!socket.can_recv(), "socket still has data to read");
                            changes.push((handle, Change::TcpClosed));
                            self.accepted_sockets.remove(&handle);
                        }
                        State::SynReceived => {
                            if self.accepted_sockets.insert(handle) {
                                let endpoint = socket.remote_endpoint().unwrap();
                                changes.push((handle, Change::TcpAccepted { endpoint }));
                            }
                        }
                        State::Listen | State::SynSent => {
                            // No changes. Do nothing.
                        }
                    }
                }
            }
        }

        if poll_result == PollResult::None && changes.is_empty() {
            let next_deadline = iface
                .poll_at(timestamp, &self.sockets)
                .map(|instant| self.instant.from_smol(instant));

            ControlFlow::Break(next_deadline)
        } else {
            ControlFlow::Continue(changes)
        }
    }
}
