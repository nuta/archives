#![no_std]
 #![feature(alloc)]


#[cfg(test)]
extern crate std;

#[macro_use]
extern crate resea;

#[cfg(not(test))]
extern crate resea_langitems;

#[macro_use]
extern crate alloc;

mod route;
mod endian;
mod packet;
mod ethernet;
mod netif;
mod arp;
mod ip;
mod ipv4;
mod transport;
mod udp;
mod socket;
mod dhcp;

pub use ipv4::{Ipv4Addr};
pub use ip::IpAddr;
pub use netif::{NetIf, NetIfType};
use arp::{Arp, MacAddr};
use route::{Routes, Route};
use ip::Network;
use socket::{Socket, Sockets, SocketInner, SocketUser};
use transport::Transport;
use dhcp::{DhcpClient, DhcpTransactionId};
use packet::{Packet, PacketInfo};
use core::cell::{Cell, RefCell};
use alloc::rc::Rc;
use alloc::collections::BTreeMap;
use resea::interfaces::net_device::{NetDevice};

#[derive(Debug)]
pub enum Error {
    DeviceError,
    NoRoute,
    Unreachable,
    MalformedPacket,
    UnknownProtocol,
    UnavailableApi,
    InvalidResponse,
    Unfinished,
}

pub type Result<T> = core::result::Result<T, Error>;

pub struct Stack {
    routes: Routes,
    arp: Arp,
    sockets: Sockets,
    dhcp_clients: RefCell<BTreeMap<DhcpTransactionId, (Rc<NetIf>, DhcpClient)>>,
    next_dhcp_xid: Cell<u32>,
}

impl Stack {
    pub fn new() -> Stack {
        let routes = Routes::new();
        let arp = Arp::new();
        let sockets = Sockets::new();
        let dhcp_clients = RefCell::new(BTreeMap::new());

        Stack {
            arp: arp,
            routes: routes,
            sockets: sockets,
            dhcp_clients: dhcp_clients,
            next_dhcp_xid: Cell::new(1),
        }
    }

    pub fn bind_udp(&self, addr: &IpAddr, port: u16) -> Rc<Socket> {
        self.sockets.add(addr, Transport::Udp, SocketUser::User, port)
    }

    pub fn add_ethernet_device(&self, device: NetDevice, dhcp: bool) {
        let hwaddr =  {
            let result = device.get_hwaddr().unwrap();
            let slice = result.as_slice();
            let mut addr = [0; 6];
            addr.clone_from_slice(slice);
            addr
        };

        let netif = Rc::new(NetIf::new(
            NetIfType::Ethernet,
            device,
            MacAddr::from_slice(&hwaddr),
        ));

        if dhcp {
            let xid = self.next_dhcp_xid.get();
            let sock = self.sockets.add(
                &IpAddr::from_ipv4_addr(Ipv4Addr::UNSPECIFIED),
                Transport::Udp,
                SocketUser::DhcpClient { xid: xid },
                68
            );

            let client = DhcpClient::new(xid, sock, netif.clone());
            self.next_dhcp_xid.replace(xid + 1);
            client.start().ok();
            self.dhcp_clients.borrow_mut().insert(xid, (netif, client));
        } else {
            // TODO: self.routes.add(netif, Ipv4Addr::UNSPECIFIED);
        }
    }

    pub fn send_packet(&self, route: Option<Route>, pktinfo: PacketInfo, mut pkt: Packet) -> Result<()> {
        // Look for the route.
        let route = if route.is_some() {
            route.unwrap()
        } else {
            try!(
                match pktinfo.network {
                    Network::Ipv4 =>
                        if let Some(route) = self.routes.route_ipv4(&pktinfo.dst.ipv4_addr()) {
                            Ok(route)
                        } else {
                            Err(Error::NoRoute)
                        },
                    _ => Err(Error::Unreachable),
                }
            )
        };

        // Construct UDP/TCP header.
        try!(
            match pktinfo.transport {
                Transport::Udp => udp::construct(&mut pkt, pktinfo.dst_port, pktinfo.src_port),
            }
        );

        // Construct IP header.
        try!(
            match pktinfo.network {
                Network::Ipv4 => ipv4::construct(&mut pkt, pktinfo.transport.clone(), pktinfo.dst.ipv4_addr(), &route.src),
                _ => Err(Error::Unreachable),
            }
        );

        try!(
            match pktinfo.network {
                Network::Ipv4 =>
                    if let Some(dst_hwaddr) = self.arp.lookup(pktinfo.dst.ipv4_addr()) {
                        route.netif.send(0x800, &dst_hwaddr, pkt)
                    } else {
                        /* The destination hardware address does not exists in the ARP cache
                           table. Enqueue the packet and send a ARP request. */
                        let arp_pkt = try!(self.arp.resolve(&route.netif, pktinfo.dst.ipv4_addr(), pkt));
                        route.netif.send(0x806, &MacAddr::BROADCAST, arp_pkt)
                    },
                _ => Err(Error::Unreachable),
            }
        );

        Ok(())
    }

    pub fn send_queued_packets(&self) {
        for sock in self.sockets.sockets.borrow().values() {
            match &sock.inner {
                SocketInner::Datagram { tx, rx: _ } => {
                    while let Some((route, pktinfo, pkt)) = tx.borrow_mut().pop() {
                        self.send_packet(route, pktinfo, pkt).ok();
                    }
                },
                _ => (),
            }
        }
    }

    pub fn poll(&self) {
        self.send_queued_packets();
    }

    pub fn receive_frame(&self, frame: &[u8]) -> Result<()> {
        let (network, _, dst_macaddr, eth_off) = try!(ethernet::receive(frame));

        let (transport, src, dst, ip_off) = try!(match network {
            Network::Ipv4 => ipv4::receive(&frame[eth_off..]),
            _ => Err(Error::UnknownProtocol),
        });

        if !self.routes.is_ours(&dst) {
            return Ok(());
        }

        let (src_port, dst_port, trans_off, len) = try!(match transport {
            Transport::Udp => udp::receive(&frame[(eth_off + ip_off)..]),
        });

        if let Some(sock) = self.sockets.sockets.borrow().get(&dst_port) {
            let trans_start = eth_off + ip_off + trans_off;
            let payload = &frame[trans_start..(trans_start + len)];

            match &sock.user {
                SocketUser::User => {
                    match &sock.inner {
                        SocketInner::Datagram { tx: _, rx } => {
                            rx.borrow_mut().push((src, src_port, payload.to_vec()));
                        }
                    }
                },
                SocketUser::DhcpClient { xid } => {
                    let mut delete = false;
                    println!("DHCP: xid={}", xid);
                    if let Some((netif, client)) = self.dhcp_clients.borrow().get(&xid) {
                        if let Ok((addr, netmask, gateway)) = client.input(payload) {
                            println!("DHCP: Done! (addr={}, netmask={}, gateway={})", addr, netmask, gateway);
                            self.routes.add_ipv4(netif.clone(), addr, netmask, gateway);
                            delete = true;
                        } else {
                            // TODO:
                        }
                    }

                    if delete {
                        self.dhcp_clients.borrow_mut().remove(&xid);
                    }
                }
            }
        }


        Ok(())
    }
}
