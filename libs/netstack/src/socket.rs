use alloc::collections::BTreeMap;
use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;
use dhcp::DhcpTransactionId;
use ip::IpAddr;
use ip::Network;
use packet::{Packet, PacketInfo};
use route::Route;
use transport::Transport;
use {Error, Result};

pub enum SocketInner {
    Datagram {
        tx: RefCell<Vec<(Option<Route>, PacketInfo, Packet)>>,
        rx: RefCell<Vec<(IpAddr, u16, Vec<u8>)>>,
    },
}

pub enum SocketUser {
    User,
    DhcpClient { xid: DhcpTransactionId },
}

pub struct Socket {
    transport: Transport,
    port: u16,
    network: Network,
    pub(crate) inner: SocketInner,
    pub(crate) user: SocketUser,
}

impl Socket {
    pub fn new(
        network: Network,
        transport: Transport,
        user: SocketUser,
        _addr: &IpAddr,
        port: u16,
    ) -> Socket {
        let inner = match transport {
            Transport::Udp => SocketInner::Datagram {
                tx: RefCell::new(Vec::new()),
                rx: RefCell::new(Vec::new()),
            },
        };

        Socket {
            transport: transport,
            port: port,
            network: network,
            inner: inner,
            user: user,
        }
    }

    pub fn sendto(&self, dst: IpAddr, dst_port: u16, data: &[u8]) -> Result<()> {
        if self.transport != Transport::Udp {
            return Err(Error::UnavailableApi);
        }

        self.do_sendto(None, dst, dst_port, data)
    }

    pub fn do_sendto(
        &self,
        route: Option<&Route>,
        dst: IpAddr,
        dst_port: u16,
        data: &[u8],
    ) -> Result<()> {
        let tx = match &self.inner {
            SocketInner::Datagram { tx, rx: _ } => tx,
            _ => return Err(Error::UnavailableApi),
        };

        let pktinfo = PacketInfo::new(
            Transport::Udp,
            self.network.clone(),
            dst,
            dst_port,
            self.port,
        );
        let pkt = Packet::new();
        pkt.set_data_from_slice(data);
        tx.borrow_mut()
            .push((route.map(|r| r.clone()), pktinfo, pkt));
        Ok(())
    }
}

pub struct Sockets {
    pub sockets: RefCell<BTreeMap<u16, Rc<Socket>>>,
}

impl Sockets {
    pub fn new() -> Sockets {
        Sockets {
            sockets: RefCell::new(BTreeMap::new()),
        }
    }

    pub fn add(
        &self,
        addr: &IpAddr,
        transport: Transport,
        user: SocketUser,
        port: u16,
    ) -> Rc<Socket> {
        let sock = Rc::new(Socket::new(
            addr.network().clone(),
            transport,
            user,
            addr.clone(),
            port,
        ));
        self.sockets.borrow_mut().insert(port, sock.clone());
        sock
    }
}
